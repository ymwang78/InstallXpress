/*******************************************************
* 文件描叙:注册表操作类;
* 日期:2015-10-28;
*******************************************************/
#ifndef _REGOPERATION_H_
#define _REGOPERATION_H_

#include <atlbase.h>
#include <string>
using namespace std;

class CRegOperation
{
public:
	CRegOperation();
	~CRegOperation();

public:
	bool CreateRegKey(HKEY hKeyRoot, LPCTSTR subkey, LPCTSTR createkey);
	bool WriteRegKey(HKEY hKeyRoot, LPCTSTR subkey, LPCTSTR pkeyname, LPCTSTR pstrvalue);
	std::wstring ReadRegKey(HKEY hKeyRoot, LPCTSTR subkey, LPCTSTR pkeyname);
	bool DeleteKeyValue(HKEY hKeyRoot, LPCTSTR subkey, LPCTSTR keyvalue);
	bool DeleteKey(HKEY hKeyRoot, LPCTSTR lkey, LPCTSTR subkey);

	LONG Open(HKEY hKeyRoot, LPCTSTR pszPath, REGSAM dwAccess = KEY_ALL_ACCESS);
	LONG Read(LPCTSTR pszKey, BYTE* pData, DWORD& dwLength);
	LONG Read(LPCTSTR pszKey, wstring& sVal);
	LONG Read(LPCTSTR pszKey, DWORD& dwVal);
	LONG Write(LPCTSTR pszKey, const BYTE* pData, DWORD dwLength);
	LONG Write(LPCTSTR pszKey, LPCTSTR pszVal);
	LONG Write(LPCTSTR pszKey, DWORD dwVal);
	LONG DelKey(LPCTSTR pszKey);
	void Close();

public:
	HKEY m_hKey;
	wstring m_sPath;
};

#endif // !_REGOPERATION_H_