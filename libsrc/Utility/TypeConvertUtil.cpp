#include "stdafx.h"
#include "TypeConvertUtil.h"


CTypeConvertUtil::CTypeConvertUtil()
{
}


CTypeConvertUtil::~CTypeConvertUtil()
{
}

wstring CTypeConvertUtil::UTF82Unicode(const string& str)
{
	int  len = 0;
	len = str.length();
	int  unicodeLen = ::MultiByteToWideChar(CP_UTF8,0,str.c_str(),-1,NULL,0);
	wchar_t *  pUnicode = NULL;
	pUnicode = new  wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1)*sizeof(wchar_t));
	::MultiByteToWideChar(CP_UTF8,0,str.c_str(),-1,(LPWSTR)pUnicode,unicodeLen);
	wstring  rt;
	rt = (wchar_t*)pUnicode;
	delete  pUnicode;
	return  rt;
}

string CTypeConvertUtil::UnicodeToUTF8(const wstring& str)
{
	char* pElementText = NULL;
	int   iTextLen;
	// wide char to multi char
	iTextLen = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.size(), NULL, 0, NULL, NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, sizeof(char) * (iTextLen + 1));
	::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.size(), pElementText, iTextLen, NULL, NULL);
	string strText(pElementText, iTextLen);
	delete[] pElementText;
	return strText;
}

void CTypeConvertUtil::ConvertUTF8ToANSI(string &strUTF8)
{
	int nwLen = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
	memset(pwBuf, 0, nwLen * 2 + 2);

	MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), strUTF8.length(), pwBuf, nwLen);

	int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	memset(pBuf, 0, nLen + 1);

	WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr = pBuf;
	strUTF8 = retStr;

	delete[]pBuf;
	delete[]pwBuf;

	pBuf = NULL;
	pwBuf = NULL;
}

void CTypeConvertUtil::ConvertANSIToUTF8(std::string& strANSI)
{
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, strANSI.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
	ZeroMemory(pwBuf, nwLen * 2 + 2);

	::MultiByteToWideChar(CP_ACP, 0, strANSI.c_str(), strANSI.length(), pwBuf, nwLen);

	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);

	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr(pBuf);
	strANSI = retStr;

	delete[]pwBuf;
	delete[]pBuf;

	pwBuf = NULL;
	pBuf = NULL;
}

std::wstring CTypeConvertUtil::StringToWstring(const std::string& strInput)
{
	int  len = 0;
	len = strInput.length();
	int  unicodeLen = ::MultiByteToWideChar(CP_ACP,0, strInput.c_str(),-1,NULL,0);
	wchar_t *  pUnicode = NULL;
	pUnicode = new  wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1)*sizeof(wchar_t));
	::MultiByteToWideChar(CP_ACP,0, strInput.c_str(),-1,	(LPWSTR)pUnicode,unicodeLen);	
	std::wstring wstr = (wchar_t*)pUnicode;
	delete  pUnicode;
	return wstr;
}

std::string CTypeConvertUtil::WstringToString(const std::wstring& strInput)
{
	char* pElementText = NULL;
	int iTextLen = 0;

	// wide char to multi char
	iTextLen = WideCharToMultiByte(CP_ACP, 0, strInput.c_str(), strInput.size(), NULL, 0, NULL, NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, sizeof(char) * (iTextLen + 1));

	INT nResult = ::WideCharToMultiByte(CP_ACP, 0, strInput.c_str(), strInput.size(), pElementText, iTextLen, NULL, NULL);
	if (nResult == 0)
	{
		delete[] pElementText;
		return "";
	}
	std::string str(pElementText, nResult);
	delete[] pElementText;
	return str;
}

void CTypeConvertUtil::strTrim(std::string& strtext)
{
	if (strtext.empty())
		return;

	std::string trimstring = " ";	
	strtext.erase(0, strtext.find_first_not_of(trimstring)); // 擦除首部空格;	
	strtext.erase(strtext.find_last_not_of(trimstring) + 1); // 擦除尾部空格;
}

void CTypeConvertUtil::charTowchar(const char *chr, wchar_t *wchar, int size)
{
	MultiByteToWideChar(CP_ACP, 0, chr, strlen(chr) + 1, wchar, size / sizeof(wchar[0]));
}

void CTypeConvertUtil::wcharTochar(const wchar_t *wchar, char *chr, int length)
{
	WideCharToMultiByte(CP_ACP, 0, wchar, -1, chr, length, NULL, NULL);
}

unsigned int CTypeConvertUtil::LpctstrToUint32(LPCTSTR lpstr)
{
	unsigned int udata = 0;
#ifdef _UNICODE
	udata = wcstoul(lpstr, NULL, 10);
#else
	udata = strtoul(lpstr, NULL, 10);
#endif
	return udata;
}

wstring CTypeConvertUtil::Uint32ToStr(unsigned int uSrc)
{
	string str = "";
	_ultoa(uSrc, (char*)str.c_str(), 10);
	wstring wstr = CTypeConvertUtil::StringToWstring(str);	
	return wstr;
}

wstring& CTypeConvertUtil::replace_all_distinct(wstring& str, const wstring& old_value, const wstring& new_value)
{
	for (wstring::size_type pos(0); pos != wstring::npos; pos += new_value.length())
	{
		if ((pos = str.find(old_value, pos)) != wstring::npos)
			str.replace(pos, old_value.length(), new_value);
		else
			break;
	}
	return str;
}