#include "stdafx.h"
#include "GeneralFormat.h"
#include <time.h>
#include <objbase.h>
#include "Md5.h"
using namespace Utility;
CGeneralFormat::CGeneralFormat()
{
}


CGeneralFormat::~CGeneralFormat()
{
}

std::string CGeneralFormat::FormatChatTime(unsigned int utime)
{
	std::string strTime = "";
	time_t temp = utime;
	struct tm* showTime = localtime(&temp);
	if (NULL != showTime && 0 < time )
	{
		char buf[128] = { 0 };
		strftime(buf, 127, "%Y/%m/%d %H:%M:%S", showTime);
		strTime = buf;
	}
	return strTime;
}


//将时间格式化为聊天需要的时间格式;
std::string CGeneralFormat::TimeToString(unsigned int utime)
{
	std::string strTime = "";
	time_t temp = utime;
	struct tm* showTime = localtime(&temp);
	if (NULL != showTime && 0 < time)
	{
		char buf[128] = { 0 };
		strftime(buf, 127, "%Y-%m-%d %H:%M:%S", showTime);
		strTime = buf;
	}
	return strTime;
}

long long CGeneralFormat::GetCurTime()
{	
	time_t rawtime;
	time(&rawtime);
	long long curtime = rawtime * 1000;
	return curtime;
}

std::string CGeneralFormat::GenerateGuid()
{
	char buf[64] = { 0 };
	GUID guid;
	if (S_OK == ::CoCreateGuid(&guid))
	{
		_snprintf(buf, sizeof(buf)
			, "%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X"
			, guid.Data1
			, guid.Data2
			, guid.Data3
			, guid.Data4[0], guid.Data4[1]
			, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
			, guid.Data4[6], guid.Data4[7]
			);
		string strGuid(buf);
		return strGuid;
	}
	return (const char*)buf;
}

//Md5;
std::string CGeneralFormat::BuildMd5(string strtext)
{
	MD5 md5;
	md5.update(strtext);
	string strmd5 = md5.toString();
	return strmd5;
}

std::string CGeneralFormat::BuildFileMd5(const wchar_t* filepath)
{
	string strfilemd5 = "";

	HANDLE h = CreateFile(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == h)
		return "";

	bool bRet = false;	
	DWORD dwReaded = 0;	
	unsigned char buffer[4096 * 8] = { 0 };

	MD5 md5;
	while (1)
	{
		if (!ReadFile(h, buffer, sizeof(buffer), &dwReaded, NULL))
			return false;
		if (0 == dwReaded)
		{
			strfilemd5 = md5.toString();
			break;
		}
		else
		{
			md5.update((void*)buffer, dwReaded);
		}
	}

	CloseHandle(h);
	return strfilemd5;
}

std::wstring CGeneralFormat::BigNumToString(UINT64 lNum)
{
	TCHAR szNum[100] = {0};
	_stprintf(szNum, TEXT("%llu"), lNum);

	NUMBERFMT nf;
	nf.NumDigits = 0;
	nf.LeadingZero = FALSE;
	nf.Grouping = 3;
	nf.lpDecimalSep = TEXT(".");
	nf.lpThousandSep = TEXT(",");
	nf.NegativeOrder = 0;

	TCHAR ostr[100] = { 0 };
	PTSTR szReturn = NULL;
	if (GetNumberFormat(LOCALE_USER_DEFAULT, 0, szNum, &nf, ostr, 100) <= 0)
		_tcscpy(ostr, szNum);

	std::wstring strbignum(ostr);
	return strbignum;
}