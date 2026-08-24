#include "stdafx.h"
#include "UnZip7z.h"
#include <Shlwapi.h>
#include "Utility/TypeConvertUtil.h"
#include "Utility/log.h"
#include "7ZLookInStream.h"
#include "Utility/ResourceHandler.h"
#include <shellapi.h>
#include <CommCtrl.h>
#include <mutex>
#pragma comment(lib, "shell32.lib")

extern "C"
{
#include "7Z/7z.h"
#include "7Z/7zAlloc.h"
#include "7Z/7zCrc.h"
#include "7Z/7zFile.h"
#include "7Z/7zVersion.h"
}

extern "C" void ExecuteProcess(const wchar_t* cmd, bool hidden, int wait_second);

namespace {

std::wstring NormalizeSeparators(std::wstring path)
{
    for (wchar_t& ch : path) {
        if (ch == L'/') {
            ch = L'\\';
        }
    }
    return path;
}

bool IsExtendedLengthPath(const std::wstring& path)
{
    return path.rfind(L"\\\\?\\", 0) == 0;
}

std::wstring ToExtendedLengthPath(const std::wstring& path)
{
    if (path.empty() || IsExtendedLengthPath(path)) {
        return path;
    }

    if (path.rfind(L"\\\\", 0) == 0) {
        return L"\\\\?\\UNC\\" + path.substr(2);
    }

    if (path.length() > 2 && path[1] == L':') {
        return L"\\\\?\\" + path;
    }

    return path;
}

size_t GetRootLength(const std::wstring& path)
{
    if (path.rfind(L"\\\\?\\UNC\\", 0) == 0) {
        size_t serverEnd = path.find(L'\\', 8);
        if (serverEnd == std::wstring::npos) {
            return path.length();
        }
        size_t shareEnd = path.find(L'\\', serverEnd + 1);
        return shareEnd == std::wstring::npos ? path.length() : shareEnd + 1;
    }

    if (path.rfind(L"\\\\?\\", 0) == 0 && path.length() > 6 && path[5] == L':' && path[6] == L'\\') {
        return 7;
    }

    if (path.rfind(L"\\\\", 0) == 0) {
        size_t serverEnd = path.find(L'\\', 2);
        if (serverEnd == std::wstring::npos) {
            return path.length();
        }
        size_t shareEnd = path.find(L'\\', serverEnd + 1);
        return shareEnd == std::wstring::npos ? path.length() : shareEnd + 1;
    }

    if (path.length() > 2 && path[1] == L':' && path[2] == L'\\') {
        return 3;
    }

    return 0;
}

bool EnsureDirectoryTree(const std::wstring& rawPath)
{
    std::wstring normalizedPath = NormalizeSeparators(rawPath);
    if (normalizedPath.empty()) {
        return false;
    }

    if (normalizedPath.back() == L'\\') {
        normalizedPath.pop_back();
    }
    if (normalizedPath.empty()) {
        return false;
    }

    std::wstring longPath = ToExtendedLengthPath(normalizedPath);
    const size_t rootLength = GetRootLength(longPath);

    for (size_t i = rootLength; i < longPath.length(); ++i) {
        if (longPath[i] != L'\\') {
            continue;
        }

        std::wstring currentPath = longPath.substr(0, i);
        if (currentPath.empty()) {
            continue;
        }

        if (!CreateDirectoryW(currentPath.c_str(), NULL)) {
            DWORD lastError = GetLastError();
            if (lastError != ERROR_ALREADY_EXISTS) {
                return false;
            }
        }
    }

    if (!CreateDirectoryW(longPath.c_str(), NULL)) {
        DWORD lastError = GetLastError();
        if (lastError != ERROR_ALREADY_EXISTS) {
            return false;
        }
    }

    return true;
}

std::wstring GetParentDirectory(const std::wstring& rawPath)
{
    std::wstring normalizedPath = NormalizeSeparators(rawPath);
    const size_t separator = normalizedPath.find_last_of(L'\\');
    if (separator == std::wstring::npos) {
        return L"";
    }
    return normalizedPath.substr(0, separator);
}

bool StartsWithNoCase(const std::wstring& text, const std::wstring& prefix)
{
    if (text.length() < prefix.length()) {
        return false;
    }

    return _wcsnicmp(text.c_str(), prefix.c_str(), prefix.length()) == 0;
}

bool ShouldSkipArchivePath(const std::wstring& archivePath, const std::vector<std::wstring>& skipPrefixes)
{
    if (skipPrefixes.empty()) {
        return false;
    }

    std::wstring normalizedArchivePath = NormalizeSeparators(archivePath);
    while (!normalizedArchivePath.empty() && normalizedArchivePath.front() == L'\\') {
        normalizedArchivePath.erase(normalizedArchivePath.begin());
    }

    for (std::wstring prefix : skipPrefixes) {
        prefix = NormalizeSeparators(prefix);
        while (!prefix.empty() && prefix.front() == L'\\') {
            prefix.erase(prefix.begin());
        }
        while (!prefix.empty() && prefix.back() == L'\\') {
            prefix.pop_back();
        }

        if (prefix.empty()) {
            continue;
        }

        if (_wcsicmp(normalizedArchivePath.c_str(), prefix.c_str()) == 0) {
            return true;
        }

        const std::wstring prefixWithSeparator = prefix + L"\\";
        if (StartsWithNoCase(normalizedArchivePath, prefixWithSeparator)) {
            return true;
        }
    }

    return false;
}

// Custom TaskDialog button id for "overwrite": force-clear blocking attributes
// (read-only/hidden/system) and remove the existing file before writing again.
constexpr int kOverwriteButtonId = 100;

// Remembered "apply to all" decision for file-write failures. Shared across all
// parallel extraction threads so that repeated failures (e.g. many read-only
// files) prompt the user only once. 0 = ask; otherwise holds IDIGNORE or
// kOverwriteButtonId.
std::mutex g_writeFailureMutex;
int g_writeFailureChoice = 0;

// Clears blocking attributes and removes the existing target so the extraction
// can recreate it. Read-only/hidden/system files are stripped to NORMAL first;
// a file that is in use (delete fails) is renamed aside and scheduled for
// deletion on reboot so the new copy can take its place now.
void ForceRemoveExistingFile(const std::wstring& longPath)
{
    DWORD attrs = GetFileAttributesW(longPath.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        return; // nothing on disk; the open failed for another reason
    }
    if (attrs & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) {
        SetFileAttributesW(longPath.c_str(), FILE_ATTRIBUTE_NORMAL);
    }
    if (DeleteFileW(longPath.c_str())) {
        return;
    }

    const std::wstring backupPath = longPath + L".ixp_old";
    SetFileAttributesW(backupPath.c_str(), FILE_ATTRIBUTE_NORMAL);
    DeleteFileW(backupPath.c_str());
    if (MoveFileExW(longPath.c_str(), backupPath.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        MoveFileExW(backupPath.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    }
}

typedef HRESULT(WINAPI* TaskDialogIndirect_t)(const TASKDIALOGCONFIG*, int*, int*, BOOL*);

// Loads comctl32 v6 (which exports TaskDialogIndirect) via an activation context
// built from shell32's embedded manifest, so the dialog works regardless of the
// host executable's own manifest. Returns true and fills *button / *applyToAll on
// success; returns false when TaskDialog is unavailable so the caller can fall back.
bool ShowWriteFailureTaskDialog(const std::wstring& filePath, int* button, bool* applyToAll)
{
    HMODULE shell32 = GetModuleHandleW(L"shell32.dll");
    bool shell32Loaded = false;
    if (shell32 == NULL) {
        shell32 = LoadLibraryW(L"shell32.dll");
        shell32Loaded = (shell32 != NULL);
    }
    if (shell32 == NULL) {
        return false;
    }

    ACTCTXW actCtx{};
    actCtx.cbSize = sizeof(actCtx);
    actCtx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID | ACTCTX_FLAG_HMODULE_VALID;
    actCtx.hModule = shell32;
    actCtx.lpResourceName = MAKEINTRESOURCEW(124); // RT_MANIFEST id referencing comctl32 v6

    HANDLE hCtx = CreateActCtxW(&actCtx);
    ULONG_PTR cookie = 0;
    bool activated = (hCtx != INVALID_HANDLE_VALUE) && ActivateActCtx(hCtx, &cookie);

    HMODULE comctl = LoadLibraryW(L"comctl32.dll");
    TaskDialogIndirect_t pTaskDialog =
        comctl ? (TaskDialogIndirect_t)GetProcAddress(comctl, "TaskDialogIndirect") : nullptr;

    bool shown = false;
    if (pTaskDialog) {
        const std::wstring content = L"\x5199\x5165\x6587\x4EF6:<" + filePath + L">\x5931\x8D25"; // write file:<path> failed

        TASKDIALOG_BUTTON buttons[] = {
            { kOverwriteButtonId, L"\x8986\x76D6" }, // overwrite (clears read-only)
            { IDRETRY,  L"\x91CD\x8BD5" },   // retry
            { IDIGNORE, L"\x5FFD\x7565" },   // ignore
            { IDABORT,  L"\x4E2D\x6B62" },   // abort
        };

        TASKDIALOGCONFIG cfg{};
        cfg.cbSize = sizeof(cfg);
        cfg.dwFlags = 0;
        cfg.pszWindowTitle = L"\x9519\x8BEF\x63D0\x793A";          // error
        cfg.pszMainIcon = TD_WARNING_ICON;
        cfg.pszMainInstruction = L"\x5199\x5165\x6587\x4EF6\x5931\x8D25"; // write file failed
        cfg.pszContent = content.c_str();
        cfg.cButtons = ARRAYSIZE(buttons);
        cfg.pButtons = buttons;
        cfg.nDefaultButton = kOverwriteButtonId;
        cfg.pszVerificationText = L"\x90FD\x6309\x6B64\x5904\x7406"; // apply to all

        int pressed = 0;
        BOOL checked = FALSE;
        if (SUCCEEDED(pTaskDialog(&cfg, &pressed, nullptr, &checked))) {
            *button = pressed;
            *applyToAll = (checked != FALSE);
            shown = true;
        }
    }

    if (comctl) FreeLibrary(comctl);
    if (activated) DeactivateActCtx(0, cookie);
    if (hCtx != INVALID_HANDLE_VALUE) ReleaseActCtx(hCtx);
    if (shell32Loaded) FreeLibrary(shell32);
    return shown;
}

// Prompts the user about a file-write failure, offering overwrite/retry/ignore/
// abort plus an "apply to all" checkbox. Honors a previously remembered "apply to
// all" decision so repeated failures don't keep interrupting the install. Returns
// kOverwriteButtonId / IDRETRY / IDIGNORE / IDABORT.
int PromptWriteFailure(const std::wstring& filePath)
{
    std::lock_guard<std::mutex> lock(g_writeFailureMutex);
    if (g_writeFailureChoice != 0) {
        return g_writeFailureChoice;
    }

    int button = 0;
    bool applyToAll = false;
    if (ShowWriteFailureTaskDialog(filePath, &button, &applyToAll)) {
        // "Ignore" and "overwrite" are safe to apply to all; remembering retry would
        // loop forever on a genuinely unwritable file, and abort terminates the
        // installer anyway.
        if (applyToAll && (button == IDIGNORE || button == kOverwriteButtonId)) {
            g_writeFailureChoice = button;
        }
        return button;
    }

    // Fallback when TaskDialog is unavailable: classic prompt without "apply to all".
    const std::wstring msg = L"\x5199\x5165\x6587\x4EF6:<" + filePath + L">\x5931\x8D25"; // write file:<path> failed
    return MessageBoxW(NULL, msg.c_str(), L"\x9519\x8BEF\x63D0\x793A", MB_ABORTRETRYIGNORE);
}

}

CUnZip7z::CUnZip7z()
{
}


CUnZip7z::~CUnZip7z()
{
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

unsigned long long CUnZip7z::GetTotalSize(const CSzArEx* db)
{
	UInt64 totalSize = 0;
	for (UInt32 i = 0; i < db->NumFiles; i++) {
		totalSize += SzArEx_GetFileSize(db, i);
	}
	return totalSize;
}

int CUnZip7z::cat_path(LPTSTR lpszPath, LPCWSTR lpSubPath)
{
	PathAppend(lpszPath, lpSubPath);
	return 0;
}

int CUnZip7z::save_file(LPCTSTR lpszFile, const void * lpBuf, DWORD dwSize)
{
	int nRet = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	if (0 == nRet)
	{
		hFile = CreateFile(lpszFile,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			nRet = 1;
		}
	}
	if (0 == nRet)
	{
		DWORD dwWrite = 0;

		WriteFile(hFile, lpBuf, dwSize, &dwWrite, 0);

		if (dwWrite != dwSize)
		{
			nRet = 1;
		}
	}
	if (INVALID_HANDLE_VALUE != hFile)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return nRet;
}

int CUnZip7z::getunzipfilenum(ResourceHandler* resHandler)
{
	int nsize = 0;
	CSzArEx db;
	SRes res;
	ISzAlloc allocImp;
	ISzAlloc allocTempImp;
	UInt16 *temp = NULL;
	size_t tempSize = 0;
	CMem7zLookInStream lookIn{ 0 };

	InitMem7zLookInStream(&lookIn, (const BYTE*)resHandler->GetData(), resHandler->GetSize());

	////////////////////////////////////////////////////////////  
	// init  
	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;

	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	CrcGenerateTable();

	SzArEx_Init(&db);

	res = SzArEx_Open(&db, &lookIn.vtbl, &allocImp, &allocTempImp);

	// No files are available if the archive cannot be opened.
	if (SZ_OK == res) {
		nsize = db.NumFiles;
	}

	SzArEx_Free(&db, &allocImp);
	SzFree(NULL, temp);

	return nsize;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
int CUnZip7z::unzip_7z_file(ResourceHandler* resHandler, const std::wstring &mUnPackPath, HWND callback, UINT Msg, UINT nNotifyID, const std::vector<std::wstring>& skipPrefixes)
{
	LPCTSTR lpszOutputPath = mUnPackPath.c_str();
	if (!FolderExist(mUnPackPath)) {
		if (FALSE == CreatedMultipleDirectory(mUnPackPath)) {
			APPLOG(Log::LOG_ERROR)("\n---unzip_7z_file: %s create faile ---\n", WtoS(mUnPackPath).c_str());
			return -1;
		}
	}

	CSzArEx db;
	SRes res;
	ISzAlloc allocImp;
	ISzAlloc allocTempImp;

    CMem7zLookInStream lookIn{ 0 };
    InitMem7zLookInStream(&lookIn, (const BYTE*)resHandler->GetData(), resHandler->GetSize());

	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;
	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	CrcGenerateTable();
	SzArEx_Init(&db);
	res = SzArEx_Open(&db, &lookIn.vtbl, &allocImp, &allocTempImp);

	if (SZ_OK != res) {
		APPLOG(Log::LOG_ERROR)("\n---unzip_7z_file: %u open zip faile ---\n", res);
		SzArEx_Free(&db, &allocImp);
		return res;
	}

	unsigned long long totalSize = GetTotalSize(&db);

	size_t tempSize = 0;
	UInt32 blockIndex = 0xFFFFFFFF; // it can have any value before first call (if outBuffer = 0)   
	Byte *outBuffer = 0;            // it must be 0 before first call for each new archive.   
	size_t outBufferSize = 0;       // it can have any value before first call (if outBuffer = 0)   

	for (unsigned int i = 0; i < db.NumFiles; ++i) {
		notify_msg_t* pNotifyMsg = new notify_msg_t{};
		pNotifyMsg->nNotifyID = nNotifyID;
		pNotifyMsg->totalFileNum = db.NumFiles;
		pNotifyMsg->currentFileIndex = i;
		pNotifyMsg->totalSize = totalSize;

		size_t offset = 0;
		size_t outSizeProcessed = 0;
		pNotifyMsg->isDir = SzArEx_IsDir(&db, i);
        size_t fileNameLen = SzArEx_GetFileNameUtf16(&db, i, NULL);
        std::vector<UInt16> fileNameBuffer(fileNameLen == 0 ? 1 : fileNameLen, 0);
        SzArEx_GetFileNameUtf16(&db, i, fileNameBuffer.data());
        std::wstring archivePath(reinterpret_cast<wchar_t*>(fileNameBuffer.data()));
        wcsncpy_s(pNotifyMsg->szFileName, _countof(pNotifyMsg->szFileName), archivePath.c_str(), _TRUNCATE);

        if (ShouldSkipArchivePath(archivePath, skipPrefixes)) {
            PostMessage(callback, Msg, nNotifyID, (LPARAM)pNotifyMsg);
            continue;
        }

		if (!pNotifyMsg->isDir) {
			res = SzArEx_Extract(&db, &lookIn.vtbl, i, &blockIndex, &outBuffer, &outBufferSize,
				&offset, &outSizeProcessed, &allocImp, &allocTempImp);
			if (res != SZ_OK) {
                APPLOG(Log::LOG_ERROR)(
                    "\n---unzip_7z_file: SzArEx_Extract error,error code : %d: %s ---\n", res,
                    pNotifyMsg->szFileName);
				break;
			}
			pNotifyMsg->currentSize += outSizeProcessed;
		}

        std::wstring outputPath = NormalizeSeparators(lpszOutputPath);
        if (!outputPath.empty() && outputPath.back() != L'\\') {
            outputPath.push_back(L'\\');
        }
        std::wstring fullPath = outputPath + NormalizeSeparators(archivePath);
        std::wstring longFullPath = ToExtendedLengthPath(fullPath);

		CSzFile outFile;
		size_t processedSize;
		if (pNotifyMsg->isDir) {
            if (!EnsureDirectoryTree(fullPath)) {
                res = SZ_ERROR_FAIL;
                APPLOG(Log::LOG_ERROR)("\n---unzip_7z_file: CreateDirectory error, dirname : %s ,last error:%lu---\n", WtoS(fullPath).c_str(), GetLastError());
                delete pNotifyMsg;
                break;
            }
			PostMessage(callback, Msg, nNotifyID, (LPARAM)pNotifyMsg);
			continue;
		}
		else {
            std::wstring parentDirectory = GetParentDirectory(fullPath);
            if (!parentDirectory.empty() && !EnsureDirectoryTree(parentDirectory)) {
                res = SZ_ERROR_FAIL;
                APPLOG(Log::LOG_ERROR)("\n---unzip_7z_file: CreateDirectory error, dirname : %s ,last error:%lu---\n", WtoS(parentDirectory).c_str(), GetLastError());
                delete pNotifyMsg;
                break;
            }
			bool forcedOverwrite = false;
			do {
				res = 0;
                DWORD dLastError = OutFile_OpenW(&outFile, longFullPath.c_str());
                if (dLastError) {
					if (dLastError == ERROR_ACCESS_DENIED) {
						// An existing read-only file blocks the overwrite; clear the
						// attribute and retry before falling back to a shell delete.
						DWORD attrs = GetFileAttributesW(longFullPath.c_str());
						if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_READONLY)) {
							SetFileAttributesW(longFullPath.c_str(), attrs & ~FILE_ATTRIBUTE_READONLY);
							dLastError = OutFile_OpenW(&outFile, longFullPath.c_str());
						}
						if (dLastError) {
							// Some tools keep files locked; try a shell delete before retrying.
							std::wstring delCommand = L"cmd /c del /F /Q \"" + fullPath + L"\"";
							ExecuteProcess(delCommand.c_str(), true, -1);
							dLastError = OutFile_OpenW(&outFile, longFullPath.c_str());
						}
					}
					if (dLastError) {
                        res = SZ_ERROR_FAIL;
                        APPLOG(Log::LOG_ERROR)("\n---unzip_7z_file: OutFile_OpenW error, filename : %s ,last error:%lu---\n", WtoS(fullPath).c_str(), dLastError);
                        int ret = PromptWriteFailure(fullPath);
                        if (ret == kOverwriteButtonId) {
                            // Force-overwrite at most once per file; if the target
                            // still can't be opened afterwards, skip it like "ignore"
                            // so a remembered "overwrite all" can't loop forever.
                            if (!forcedOverwrite) {
                                forcedOverwrite = true;
                                ForceRemoveExistingFile(longFullPath);
                                continue;
                            }
                            APPLOG(Log::LOG_ERROR)(
                                "\n---unzip_7z_file: force overwrite failed, skip file : %s ---\n",
                                WtoS(fullPath).c_str());
                            break;
                        }
                        else if (ret == IDRETRY) {
                            continue;
                        }
                        else if (ret == IDIGNORE) {
                            break;
                        }
                        else {
                            exit(0);
                        }
					}
                }
			} while (res == SZ_ERROR_FAIL);
			if (res == SZ_ERROR_FAIL) {
                processedSize = outSizeProcessed;
                continue;
			}
		}
		processedSize = outSizeProcessed;

		if (File_Write(&outFile, outBuffer + offset, &processedSize) != 0 || processedSize != outSizeProcessed) {
			res = SZ_ERROR_FAIL;
			APPLOG(Log::LOG_ERROR)("\n---unzip_7z_file: File_Write error --\n");
			break;
		}

		if (File_Close(&outFile)) {
			res = SZ_ERROR_FAIL;
			APPLOG(Log::LOG_ERROR)("\n---unzip_7z_file: File_Close error --\n");
			break;
		}

#ifdef USE_WINDOWS_FILE
		if (SzBitWithVals_Check(&db.Attribs, i))
			SetFileAttributesW(longFullPath.c_str(), db.Attribs.Vals[i]);
#endif
		PostMessage(callback, Msg, nNotifyID, (LPARAM)pNotifyMsg);
	}

	IAlloc_Free(&allocImp, outBuffer);

	SzArEx_Free(&db, &allocImp);
	return res;
}

/////////////////////////////////////////////////////////////////////////////
// Check whether a directory exists.
// Input: strPath is the directory path to check.
// Return: TRUE when it exists, otherwise FALSE.
/////////////////////////////////////////////////////////////////////////////
BOOL CUnZip7z::FolderExist(const std::wstring& strPath)
{
	std::wstring sCheckPath = strPath;

	if (sCheckPath[sCheckPath.length() - 1] != '\\')
	{
		sCheckPath.append(1, '\\');
	}
	sCheckPath += L"*.*";

	WIN32_FIND_DATA wfd;
	BOOL rValue = FALSE;

	HANDLE hFind = FindFirstFile(sCheckPath.c_str(), &wfd);

	if ((hFind != INVALID_HANDLE_VALUE) &&
		(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) || (wfd.dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE))
	{
		// Existing directory (or an existing file matched by *.*).
		rValue = TRUE;
	}

	FindClose(hFind);
	return rValue;
}

/////////////////////////////////////////////////////////////////////////////
// Create nested directories.
// Input: Directoryname is the target path.
// Return: TRUE on success, FALSE on failure.
/////////////////////////////////////////////////////////////////////////////
BOOL CUnZip7z::CreatedMultipleDirectory(std::wstring Directoryname)
{
    return EnsureDirectoryTree(Directoryname) ? TRUE : FALSE;
}
