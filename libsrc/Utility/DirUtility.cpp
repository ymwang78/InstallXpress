#include "stdafx.h"
#include "DirUtility.h"

#include <direct.h>
#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")

CDirUtility::CDirUtility()
{
	wchar_t szFileName[MAX_PATH] = { 0 };
	wchar_t szFileDescription[128] = { 0 };
	wchar_t szCompanyName[128] = {0};
	wchar_t szProductName[128] = { 0 };
	wchar_t szVersion[128] = {0};
	::GetModuleFileName(NULL, szFileName, MAX_PATH);
	GetFileDescriptionAndProductVersion(szFileName, szFileDescription, 128, szCompanyName, 128, szProductName, 128, szVersion, 128);
	m_strFileDescription = szFileDescription;
	m_strCompanyName = szCompanyName;
	m_strProductName = szProductName;
	m_strVersion = szVersion;
}


CDirUtility::~CDirUtility()
{
}

//获取用户开始/程序;
std::string CDirUtility::GetUserStartProgramPath()
{
	char szLocalPsName[MAX_PATH] = { 0 };
	std::string strstarmenupath = "";

	LPITEMIDLIST items = NULL;
	if (SHGetSpecialFolderLocation(NULL, CSIDL_PROGRAMS, &items) == S_OK)
	{
		BOOL flag = SHGetPathFromIDListA(items, szLocalPsName);
		CoTaskMemFree(items);
		strstarmenupath = szLocalPsName;
	}
	return strstarmenupath;
}

//获取开始/程序;
std::string CDirUtility::GetStartProgramPath()
{
	char szLocalPsName[MAX_PATH] = { 0 };
	std::string strstarmenupath = "";

	LPITEMIDLIST items = NULL;
	if (SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_PROGRAMS, &items) == S_OK)
	{
		BOOL flag = SHGetPathFromIDListA(items, szLocalPsName);
		CoTaskMemFree(items);
		strstarmenupath = szLocalPsName;
	}
	return strstarmenupath;
}

//获取桌面路径;
std::string CDirUtility::GetDesktopPath()
{
	char szLocalPsName[MAX_PATH] = { 0 };
	std::string strdesktoppath = "";

	LPITEMIDLIST items = NULL;
	if (SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &items) == S_OK)
	{
		BOOL flag = SHGetPathFromIDListA(items, szLocalPsName);
		CoTaskMemFree(items);
		strdesktoppath = szLocalPsName;
	}
	return strdesktoppath;
}

//获取程序所在目录(不含程序名);
std::string CDirUtility::GetProgramInPath()
{
	char szLocalPsName[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szLocalPsName, 255);

	std::string localPsName(szLocalPsName);
	int pos = localPsName.find_last_of('\\');
	localPsName = localPsName.substr(0, pos);
	return localPsName;
}

//获取程序名称;
std::string CDirUtility::GetProgramName()
{
	char szLocalPsName[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szLocalPsName, 255);

	std::string localPsName(szLocalPsName);
	int pos = localPsName.find_last_of('\\') + 1;
	localPsName = localPsName.substr(pos, localPsName.length() - pos);
	return localPsName;
}

//获取资源目录;
std::string CDirUtility::GetResDir()
{
	char szLocalPsName[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szLocalPsName, 255);

	std::string localPsName(szLocalPsName);
	int pos = localPsName.find_last_of('\\');
	localPsName = localPsName.substr(0, pos);

	std::string strlocalPsName(localPsName);
	pos = strlocalPsName.find_last_of('\\') + 1;
	strlocalPsName = strlocalPsName.substr(0, pos);

	strlocalPsName += "SkinRes\\";

	return strlocalPsName;
}

//获取程序全路径;
std::string CDirUtility::GetProgramPath()
{
	char szLocalPsName[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szLocalPsName, 255);
	return szLocalPsName;
}

//判断是否是文件夹;
bool CDirUtility::IsDir(string szDir)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFileA(szDir.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) return true;
	if (FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
	{
		return true;
	}
	return false;
}

void CDirUtility::DeleteAllFile(std::string strFile, bool bRemoveDir)
{
	if (strFile[strFile.length() - 1] != '\\')
	{
		strFile.append(1, '\\');
	}
	string szDir = strFile + "*.*";

	BOOL bRet;
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFileA(szDir.c_str(), &FindFileData);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		bRet = ::FindNextFileA(hFind, &FindFileData);
		if (!bRet) break;

		if (FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && !(FindFileData.cFileName[0] == '.'))
		{
			//如果是一个子目录，用递归继续往深一层找 ;
			string strPath(FindFileData.cFileName);
			strPath = strFile + strPath;
			DeleteAllFile(strPath, bRemoveDir);
		}
		else if (!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && !(FindFileData.cFileName[0] == '.'))
		{
			//显示当前访问的文件(完整路径) ;
			string strPath(FindFileData.cFileName);
			strPath = strFile + strPath;
			DeleteFileA(strPath.c_str());
		}
	}
	if (strFile[strFile.length() - 1] == '\\')
	{
		strFile = strFile.substr(0, strFile.length() - 1);
	}
	FindClose(hFind);
	SetFileAttributesA(strFile.c_str(), FILE_ATTRIBUTE_NORMAL);
	if(bRemoveDir) RemoveDirectoryA(strFile.c_str());
}

//获取系统程序文件夹;
std::wstring CDirUtility::GetProgramDir()
{
	wchar_t szLocalPsName[MAX_PATH] = { 0 };
	//GetModuleFileNameA(NULL, szLocalPsName, 255);

	std::wstring strstarmenupath;
	LPITEMIDLIST items = NULL;
	if (SHGetSpecialFolderLocation(NULL, CSIDL_PROGRAM_FILES, &items) == S_OK)
	{
		BOOL flag = SHGetPathFromIDList(items, szLocalPsName);
		CoTaskMemFree(items);
		strstarmenupath = szLocalPsName;
		strstarmenupath += L"\\";
	}
	return strstarmenupath;
}

//获取AppData;
std::wstring CDirUtility::GetAppDataDir()
{
	wchar_t path[MAX_PATH] = { 0 };
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
	wstring strappdatadir = path;
	strappdatadir += L"\\" + m_strCompanyName + L"\\" + m_strProductName + L"\\";
	if (!PathIsDirectory(strappdatadir.c_str()))
		::CreateDirectory(strappdatadir.c_str(), NULL);
	return strappdatadir;
}

//获取AppData;
std::string CDirUtility::GetAppDataChatDir()
{
	char path[MAX_PATH] = { 0 };
	SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path);
	string strappdatadir = path;
	strappdatadir += "\\YueXiang\\vxchat";
	if (!PathIsDirectoryA(strappdatadir.c_str()))
		::CreateDirectoryA(strappdatadir.c_str(), NULL);
	return strappdatadir;
}

void CDirUtility::CreateMulitDir(const wstring &strdir)
{
	int ipathLength = strdir.length();
	int ileaveLength = 0;
	int iCreatedLength = 0;
	wchar_t szPathTemp[MAX_PATH] = { 0 };
	for (int i = 0;(NULL != wcschr(strdir.c_str() + iCreatedLength, '\\'));i++)
	{
		ileaveLength = wcslen(wcschr(strdir.c_str() + iCreatedLength, '\\')) - 1;
		iCreatedLength = ipathLength - ileaveLength;
		wcsncpy(szPathTemp, strdir.c_str(), iCreatedLength);
		_wmkdir(szPathTemp);
	}
	if (iCreatedLength < ipathLength)
	{
		_wmkdir(strdir.c_str());
	}
}

bool CDirUtility::CheckFileInvalid(const string &strFilePath)
{
	if (!PathFileExistsA(strFilePath.c_str()))
		return false;

	HANDLE file = CreateFileA(strFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == file)
		return false;

	string strExtenName = "";
	// 此处只处理了4G以下的文件;
	DWORD hig = 0;
	DWORD low = GetFileSize(file, &hig);
	unsigned char* buffer = new unsigned char[low];
	DWORD numberOfRead = 0;
	BOOL ret = ReadFile(file, buffer, low, &numberOfRead, NULL);
	if (ret && numberOfRead >= 4)
	{
		strExtenName = GetFileSteamExtenName(buffer);
	}
	delete[]buffer;
	CloseHandle(file);

	if (strExtenName.empty()) return false;
	return true;
}

//通过流获取扩展名;
string CDirUtility::GetFileSteamExtenName(unsigned char* pImagedata)
{
	string strExtenName = "";
	if (pImagedata[0] == 'B' && pImagedata[1] == 'M')
		strExtenName = "bmp";
	else if (pImagedata[1] == 'P' && pImagedata[2] == 'N' && pImagedata[3] == 'G')
		strExtenName = "png";
	else if (pImagedata[0] == 0xFF && pImagedata[1] == 0xD8)
	{
		if (pImagedata[7] == 'x' && pImagedata[8] == 'i') strExtenName = "";
		else strExtenName = "jpg";
	}
	return strExtenName;
}


//获取临时目录(appdir追加的目录);
std::wstring CDirUtility::GetProductTempPath(const std::wstring&strfilename)
{
	wchar_t tempdir[MAX_PATH] = { 0 };
	memset(tempdir, 0, MAX_PATH);
	::GetTempPath(MAX_PATH, tempdir);

	std::wstring strtempdir = std::wstring(tempdir) + m_strCompanyName + L"." + m_strProductName + L"\\";
	if (strfilename.find_last_of('/') != std::wstring::npos)
	{
		int pos = strfilename.find_last_of('/');
		std::wstring strsubdir = strfilename.substr(0, pos);
		CTypeConvertUtil::replace_all_distinct(strsubdir, L"/", L"\\");
		strtempdir.append(strsubdir);
	}	
	CreateMulitDir(strtempdir);

	if (strfilename.find_last_of('/') != std::string::npos)
	{
		int pos = strfilename.find_last_of('/');
		auto strsubdir = strfilename.substr(pos + 1, strfilename.length() - pos - 1);
		strtempdir.append(L"\\");
		strtempdir.append(strsubdir);
	}
	else
	{
		strtempdir.append(strfilename);
	}
	return strtempdir;
}

int CDirUtility::GetFileDescriptionAndProductVersion(LPCWSTR lpstrFilename,
	wchar_t* lpoutFileDescription, UINT cbFileDescriptionSize,
    wchar_t* lpoutCompanyName, UINT cbCompanySize,
    wchar_t* lpoutProductName, UINT cbProductSize,
    wchar_t* lpoutProductVersion, UINT cbVerionSize)
{
	typedef DWORD(WINAPI *pfnGetFileVersionInfoSize)(LPCTSTR, LPDWORD);
	typedef BOOL(WINAPI *pfnGetFileVersionInfo)(LPCTSTR, DWORD, DWORD, LPVOID);
	typedef BOOL(WINAPI *pfnVerQueryValue)(const LPVOID, LPCTSTR, LPVOID*, PUINT);

	HMODULE hDll = 0;
	DWORD   dwHandle = 0;
	DWORD   dwInfoSize = 0;

	pfnGetFileVersionInfoSize   fnGetFileVersionInfoSize = 0;
	pfnGetFileVersionInfo       fnGetFileVersionInfo = 0;
	pfnVerQueryValue            fnVerQueryValue = 0;

	*lpoutCompanyName = 0;
	*lpoutProductName = 0;
	*lpoutProductVersion = 0;

	// Load system32\version.dll module
	hDll = LoadLibrary(L"version.dll");
	if (!hDll)
		return -1;

	fnGetFileVersionInfoSize = (pfnGetFileVersionInfoSize)GetProcAddress(hDll, "GetFileVersionInfoSizeW");
	fnGetFileVersionInfo = (pfnGetFileVersionInfo)GetProcAddress(hDll, "GetFileVersionInfoW");
	fnVerQueryValue = (pfnVerQueryValue)GetProcAddress(hDll, "VerQueryValueW");

	// If failed  to get procdure address
	if (!fnGetFileVersionInfoSize || !fnGetFileVersionInfo || !fnVerQueryValue) {
		FreeLibrary(hDll);
		return -2;
	}

	dwInfoSize = fnGetFileVersionInfoSize(lpstrFilename, &dwHandle);
	if (dwInfoSize <= 0) {
        FreeLibrary(hDll);
        return -3;
	}
	
	void *pvInfo = malloc(dwInfoSize);
	if (!pvInfo) {
		FreeLibrary(hDll);
        return -4; // Out of memory
	}
	if (!fnGetFileVersionInfo(lpstrFilename, 0, dwInfoSize, pvInfo)) {
        free(pvInfo);
        FreeLibrary(hDll);
        return -5;
	}
	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	// Read the list of languages and code pages.
	UINT cbTranslate = 0;
	if (!fnVerQueryValue(pvInfo, L"\\VarFileInfo\\Translation", (void**)&lpTranslate, &cbTranslate)
		|| cbTranslate < sizeof(struct LANGANDCODEPAGE)) {
        free(pvInfo);
        FreeLibrary(hDll);
        return -6;
	}
	// ONLY Read the file description for FIRST language and code page.
	const wchar_t*lpBuffer = 0;
	UINT  cbSizeBuf = 0;
	wchar_t  szSubBlock[50];
    // Retrieve file FileDescription for language and code page 0
    wsprintf(szSubBlock, L"\\StringFileInfo\\%04x%04x\\FileDescription", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
    if (fnVerQueryValue(pvInfo, szSubBlock, (void**)&lpBuffer, &cbSizeBuf)) {
        wcsncpy_s(lpoutFileDescription, cbFileDescriptionSize, lpBuffer, cbFileDescriptionSize - 1);
		lpoutFileDescription[cbFileDescriptionSize - 1] = 0;
    }
    // Retrieve file CompanyName for language and code page 0
    wsprintf(szSubBlock, L"\\StringFileInfo\\%04x%04x\\CompanyName", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
    if (fnVerQueryValue(pvInfo, szSubBlock, (void**)&lpBuffer, &cbSizeBuf)) {
        wcsncpy_s(lpoutCompanyName, cbCompanySize, lpBuffer, cbCompanySize - 1);
		lpoutCompanyName[cbCompanySize - 1] = 0;
    }
	// Retrieve file ProductName for language and code page 0
	wsprintf(szSubBlock, L"\\StringFileInfo\\%04x%04x\\ProductName", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
	if (fnVerQueryValue(pvInfo, szSubBlock, (void**)&lpBuffer, &cbSizeBuf)) {
		wcsncpy_s(lpoutProductName, cbProductSize, lpBuffer, cbProductSize - 1);
		lpoutProductName[cbProductSize - 1] = 0;
	}
	// Retrieve file version for language and code page 0
	wsprintf(szSubBlock, L"\\StringFileInfo\\%04x%04x\\ProductVersion", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
	if (fnVerQueryValue(pvInfo, szSubBlock, (void**)&lpBuffer, &cbSizeBuf)) {
		wcsncpy_s(lpoutProductVersion, cbVerionSize, lpBuffer, cbVerionSize - 1);
		lpoutProductVersion[cbVerionSize - 1] = 0;
	}
	free(pvInfo);
	FreeLibrary(hDll);
	return 0;
}

//获取程序上一级目录;
std::wstring CDirUtility::GetProgramUpDir()
{
    TCHAR szLocalPsName[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szLocalPsName, 255);

    std::wstring localPsName(szLocalPsName);
    int pos = localPsName.find_last_of('\\');
    localPsName = localPsName.substr(0, pos);

    return localPsName;
}
//获取程序上一级目录;
std::wstring CDirUtility::GetProgramPriovDir()
{
	TCHAR szLocalPsName[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szLocalPsName, 255);

	std::wstring localPsName(szLocalPsName);
	int pos = localPsName.find_last_of('\\');
	localPsName = localPsName.substr(0, pos);

	std::wstring strlocalPsName(localPsName);
	pos = strlocalPsName.find_last_of('\\') + 1;
	strlocalPsName = strlocalPsName.substr(0, pos);
	return strlocalPsName;
}