/**********************************************************
* �ļ�����: ����ת��;
* �ļ�����  typeconvertutil.h;
*
***********************************************************/
#ifndef TYPECONVERTUTIL_H_
#define TYPECONVERTUTIL_H_

#define CheckIsZero(num)	(num == 0 ? false : true)
#define WtoS(str)			CTypeConvertUtil::WstringToString(str)
#define StoW(str)			CTypeConvertUtil::StringToWstring(str)
#define WstrToUInt(str)		CTypeConvertUtil::LpctstrToUint32(str)
#define UIntToWstr(unum)	CTypeConvertUtil::Uint32ToStr(unum)
#define Utf8ToAnsi(str)		CTypeConvertUtil::ConvertUTF8ToANSI(str)
#define AnsiToUtf8(str)		CTypeConvertUtil::ConvertANSIToUTF8(str)
#define UnicodeToUtf8(str)  CTypeConvertUtil::UnicodeToUTF8(str)
#define Utf82Unicode(str)  CTypeConvertUtil::Utf82Unicode(str)

#include <string>
using namespace std;

class CTypeConvertUtil
{
public:
	CTypeConvertUtil();
	~CTypeConvertUtil();
	
public:	
	static string  UnicodeToUTF8(const wstring& str);						
	static wstring Utf82Unicode(const string& str);
	static void ConvertUTF8ToANSI(string &strUTF8);							// UTF8תasci;	
	static void ConvertANSIToUTF8(string& strANSI);							// asciתUTF8;
	static std::wstring StringToWstring(const std::string& strInput );		// stringתwstring;
	static std::string  WstringToString(const std::wstring& strInput);		// wstirngתstring;
	static void strTrim(std::string& str);									//ȥ��ǰ��ո�;
	static void charTowchar(const char *chr, wchar_t *wchar, int size);
	static void wcharTochar(const wchar_t *wchar, char *chr, int length);
	static unsigned int LpctstrToUint32(LPCTSTR lpstr);
	static wstring Uint32ToStr(unsigned int uSrc);
	static wstring& replace_all_distinct(wstring& str, const wstring& old_value, const wstring& new_value);
	
};

#endif // TYPECONVERTUTIL_H_