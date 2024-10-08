#pragma once

class ResourceHandler;
typedef struct _CSzArEx CSzArEx;

struct notify_msg_t {
	int nNotifyID;
	int totalFileNum;
	int currentFileIndex;
	unsigned long long totalSize;
	unsigned long long currentSize;
	int isDir;
	WCHAR szFileName[MAX_PATH];
};

class CUnZip7z
{
public:
	CUnZip7z();
	~CUnZip7z();

public:
	BOOL CreatedMultipleDirectory(std::wstring Directoryname);
	BOOL FolderExist(const std::wstring& strPath);

public:
	unsigned long long GetTotalSize(const CSzArEx* db);
	int cat_path(LPTSTR lpszPath, LPCWSTR lpSubPath);
	int save_file(LPCTSTR lpszFile, const void * lpBuf, DWORD dwSize);
	int unzip_7z_file(ResourceHandler* resHandler,const std::wstring &mUnPackPath, HWND callback, UINT Msg, UINT nNotifyID);
	int getunzipfilenum(ResourceHandler* resHandler);
};
