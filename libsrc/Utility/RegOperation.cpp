#include "stdafx.h"
#include "RegOperation.h"


CRegOperation::CRegOperation()
	:m_sPath(_T(""))
{
	m_hKey = NULL;
}


CRegOperation::~CRegOperation()
{
	Close();
}

bool CRegOperation::CreateRegKey(HKEY hKeyRoot, LPCTSTR subkey, LPCTSTR createkey)
{	
	HKEY hKey;
	HKEY hTempKey;
	if (ERROR_SUCCESS == ::RegOpenKeyEx(hKeyRoot, subkey, 0, KEY_SET_VALUE, &hKey))
	{
		// 使用hKey来操作data_Set这个KEY里面的值。
		if (ERROR_SUCCESS != ::RegCreateKey(hKey, createkey, &hTempKey))
		{
			return false;
		}
	}
	::RegCloseKey(hKey);
	return true;
}

bool CRegOperation::WriteRegKey(HKEY hKeyRoot, LPCTSTR subkey, LPCTSTR pkeyname, LPCTSTR pstrvalue)
{
	HKEY hKey;
	if (ERROR_SUCCESS == ::RegOpenKeyEx(hKeyRoot, subkey, 0, KEY_SET_VALUE, &hKey))
	{
		wstring str(pstrvalue);
		if (ERROR_SUCCESS != ::RegSetValueEx(hKey, pkeyname, 0, REG_SZ, (LPBYTE)pstrvalue, str.length() * sizeof(wchar_t)))
		{
			::RegCloseKey(hKey);
			return false;
		}
	}
	::RegCloseKey(hKey);
	return true;
}

wstring CRegOperation::ReadRegKey(HKEY hKeyRoot, LPCTSTR subkey, LPCTSTR pkeyname)
{
	wstring strvalue = _T("");
	HKEY hKEY;
	if (ERROR_SUCCESS == ::RegOpenKeyEx(hKeyRoot, subkey, 0, KEY_READ, &hKEY))
	{
		TCHAR dwValue[256] = {0};
		DWORD dwSize = sizeof(dwValue);
		DWORD dwType = REG_SZ;
		::RegQueryValueEx(hKEY, pkeyname, 0, &dwType, (LPBYTE)&dwValue, &dwSize);
		::RegCloseKey(hKEY);
		strvalue = dwValue;
	}
	return strvalue;
}

bool CRegOperation::DeleteKeyValue(HKEY hKeyRoot, LPCTSTR subkey, LPCTSTR keyvalue)
{
	HKEY hKey;
	if (ERROR_SUCCESS == ::RegOpenKeyEx(hKeyRoot, subkey, 0, KEY_SET_VALUE, &hKey))
	{
		if (ERROR_SUCCESS != ::RegDeleteValue(hKey, keyvalue))
		{
			::RegCloseKey(hKey);
			return false;
		}
	}
	::RegCloseKey(hKey);
	return true;
}

bool  CRegOperation::DeleteKey(HKEY hKeyRoot, LPCTSTR lkey, LPCTSTR subkey)
{
	HKEY hKey;
	if (ERROR_SUCCESS == ::RegOpenKeyEx(hKeyRoot, lkey, 0, KEY_ALL_ACCESS, &hKey))
	{
		if (ERROR_SUCCESS != ::RegDeleteKey(hKey, subkey))
		{
			DWORD errorcode = GetLastError();
			::RegCloseKey(hKey);
			return false;
		}
	}
	::RegCloseKey(hKey);
	return true;
}

LONG CRegOperation::Open(HKEY hKeyRoot, LPCTSTR pszPath, REGSAM dwAccess/* = KEY_ALL_ACCESS*/)
{
	DWORD dw;
	m_sPath = pszPath;

	return RegCreateKeyEx(hKeyRoot, pszPath, 0L, NULL, REG_OPTION_NON_VOLATILE, dwAccess, NULL, &m_hKey, &dw);
}


LONG CRegOperation::Read(LPCTSTR pszKey, BYTE* pData, DWORD& dwLength)
{
	ATLASSERT(m_hKey);
	ATLASSERT(pszKey);
	DWORD dwType;
	return RegQueryValueEx(m_hKey, pszKey, NULL, &dwType, pData, &dwLength);
}

LONG CRegOperation::Read(LPCTSTR pszKey, wstring& sVal)
{
	ATLASSERT(m_hKey);
	ATLASSERT(pszKey);

	DWORD dwType;
	DWORD dwSize = 512;
	TCHAR string[512] = { 0 };

	LONG lRet = RegQueryValueEx(m_hKey, pszKey, NULL, &dwType, (LPBYTE)string, &dwSize);

	if (ERROR_SUCCESS == lRet)
	{
		sVal = string;
	}

	return lRet;
}

LONG CRegOperation::Read(LPCTSTR pszKey, DWORD& dwVal)
{
	ATLASSERT(m_hKey);
	ATLASSERT(pszKey);

	DWORD dwType;
	DWORD dwSize = sizeof(DWORD);
	DWORD dwDest;

	LONG lRet = RegQueryValueEx(m_hKey, pszKey, NULL, &dwType, (LPBYTE)&dwDest, &dwSize);

	if (ERROR_SUCCESS == lRet)
	{
		dwVal = dwDest;
	}

	return lRet;
}

LONG CRegOperation::Write(LPCTSTR pszKey, const BYTE* pData, DWORD dwLength)
{
	ATLASSERT(m_hKey);
	ATLASSERT(pszKey);
	ATLASSERT(pData && dwLength > 0);

	return RegSetValueEx(m_hKey, pszKey, 0L, REG_BINARY, pData, dwLength);
}

LONG CRegOperation::Write(LPCTSTR pszKey, LPCTSTR pszVal)
{
	ATLASSERT(m_hKey);
	ATLASSERT(pszKey);
	ATLASSERT(pszVal);

	return RegSetValueEx(m_hKey, pszKey, 0L, REG_SZ, (const BYTE*)pszVal, wcslen(pszVal) * 2 + sizeof(TCHAR));
}

LONG CRegOperation::Write(LPCTSTR pszKey, DWORD dwVal)
{
	ATLASSERT(m_hKey);
	ATLASSERT(pszKey);

	return RegSetValueEx(m_hKey, pszKey, 0L, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
}

LONG CRegOperation::DelKey(LPCTSTR pszKey)
{
	ATLASSERT(m_hKey);
	ATLASSERT(pszKey);

	return RegDeleteValue(m_hKey, pszKey);
}

void CRegOperation::Close()
{
	if (m_hKey)
	{
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}
}