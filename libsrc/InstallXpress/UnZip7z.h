/**********************************************
* �ļ���:UnZip7z.h;
* �ļ�����:7z��ѹ;
***********************************************/
#ifndef  _UNZIP7Z_H_
#define  _UNZIP7Z_H_

class ResourceHandler;

class CUnZip7z
{
public:
	CUnZip7z();
	~CUnZip7z();

public:
	//����·�� ;
	BOOL CreatedMultipleDirectory(std::wstring Directoryname);
	BOOL FolderExist(std::wstring& strPath);

public:
	int cat_path(LPTSTR lpszPath, LPCWSTR lpSubPath);
	int save_file(LPCTSTR lpszFile, const void * lpBuf, DWORD dwSize);
	int unzip_7z_file(ResourceHandler* resHandler, std::wstring &mUnPackPath, HWND callback, UINT Msg);
	int getunzipfilenum(ResourceHandler* resHandler);
};

#endif // ! _UNZIP7Z_H_