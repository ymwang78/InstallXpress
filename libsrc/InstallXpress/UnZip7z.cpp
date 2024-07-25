#include "stdafx.h"
#include "UnZip7z.h"
#include <Shlwapi.h>
#include "Utility/TypeConvertUtil.h"
#include "Utility/log.h"
#include "7ZLookInStream.h"
#include "Utility/ResourceHandler.h"
#include <shellapi.h>
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

	// 如果没有打开成功,就不会有文件; 
	if (SZ_OK == res) {
		nsize = db.NumFiles;
	}

	SzArEx_Free(&db, &allocImp);
	SzFree(NULL, temp);

	return nsize;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
int CUnZip7z::unzip_7z_file(ResourceHandler* resHandler, std::wstring &mUnPackPath, HWND callback, UINT Msg, UINT nNotifyID)
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
		size_t len;
		pNotifyMsg->isDir = SzArEx_IsDir(&db, i);
		len = SzArEx_GetFileNameUtf16(&db, i, (UInt16*)pNotifyMsg->szFileName);

		if (!pNotifyMsg->isDir) {
			res = SzArEx_Extract(&db, &lookIn.vtbl, i, &blockIndex, &outBuffer, &outBufferSize,
				&offset, &outSizeProcessed, &allocImp, &allocTempImp);
			if (res != SZ_OK) {
				APPLOG(Log::LOG_ERROR)("\n---unzip_7z_file: SzArEx_Extract error,error code : %d ---\n", res);
				break;
			}
			pNotifyMsg->currentSize += outSizeProcessed;
		}

		TCHAR szFile[MAX_PATH] = { 0 };
		_tcscpy_s(szFile, lpszOutputPath);

		CSzFile outFile;
		size_t processedSize;
		PathAppend(szFile, (LPCWSTR)pNotifyMsg->szFileName);

		for (size_t j = 0; szFile[j] != 0; j++) {
			if ((szFile[j] == '/') || (szFile[j] == '\\')) {
				szFile[j] = 0;
				CreateDirectory(szFile, 0);
				szFile[j] = CHAR_PATH_SEPARATOR;
			}
		}
		if (pNotifyMsg->isDir) {
			CreateDirectory(szFile, 0);
			PostMessage(callback, Msg, nNotifyID, (LPARAM)pNotifyMsg);
			continue;
		}
		else {
			do {
				res = 0;
                DWORD dLastError = OutFile_OpenW(&outFile, szFile);
                if (dLastError) {
					if (dLastError == 5) {
						//有的程序例如HASP会拒绝其他进程访问，先用SHELL去删除
                        CDuiString delCommand;
						delCommand.Format(L"cmd /c del /F /Q %s", szFile);
                        ExecuteProcess(delCommand, true, -1);
						dLastError = OutFile_OpenW(&outFile, szFile);
					}
					if (dLastError) {
                        res = SZ_ERROR_FAIL;
                        APPLOG(Log::LOG_ERROR)("\n---unzip_7z_file: OutFile_OpenW error, filename : %s ,last error:%lu---\n", WtoS(szFile).c_str(), dLastError);
                        CDuiString msg;
                        msg.Format(_T("写入文件:<%s>失败"), szFile);
                        int ret = MessageBox(NULL, msg, _T("错误提示"), MB_ABORTRETRYIGNORE);
                        if (ret == IDRETRY) {
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
			SetFileAttributesW(szFile, db.Attribs.Vals[i]);
#endif
		PostMessage(callback, Msg, nNotifyID, (LPARAM)pNotifyMsg);
	}

	IAlloc_Free(&allocImp, outBuffer);

	SzArEx_Free(&db, &allocImp);
	return res;
}

///////////////////////////////////////////////////////////////////////////// 
// 函数说明: 检查指定的文件夹是否存在 ;
// 参数说明: [in]：strPath 检查的文件夹 (此方法会主动向路径末尾添加*.*) ;
// 返回值:BOOL类型,存在返回TRUE,否则为FALSE ;
///////////////////////////////////////////////////////////////////////////// 
BOOL CUnZip7z::FolderExist(std::wstring& strPath)
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
		//如果存在并类型是文件夹 ;
		rValue = TRUE;
	}

	FindClose(hFind);
	return rValue;
}

///////////////////////////////////////////////////////////////////////////// 
// 函数说明: 创建多级目录 ;
// 参数说明: [in]： 路径字符串 ;
// 返回值: BOOL 成功True 失败False ;///////////////////////////////////////////////////////////////////////////// 
BOOL CUnZip7z::CreatedMultipleDirectory(std::wstring Directoryname)
{
	if (Directoryname[Directoryname.length() - 1] != '\\')
	{
		Directoryname.append(1, '\\');
	}
	std::vector< std::wstring> vpath;
	std::wstring strtemp;
	BOOL  bSuccess = FALSE;
	for (size_t i = 0; i < Directoryname.length(); i++)
	{
		if (Directoryname[i] != '\\')
		{
			strtemp.append(1, Directoryname[i]);
		}
		else
		{
			vpath.push_back(strtemp);
			strtemp.append(1, '\\');
		}
	}
	std::vector<std::wstring>::const_iterator vIter;
	for (vIter = vpath.begin();vIter != vpath.end(); vIter++)
	{
		bSuccess = CreateDirectory(vIter->c_str(), NULL) ? TRUE : FALSE;
	}

	return bSuccess;
}
