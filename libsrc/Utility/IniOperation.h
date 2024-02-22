
#ifndef INIOPERATION__H__
#define INIOPERATION__H__

#include <atlbase.h>
#include <string>
using namespace std;

class CIniReader
{
public:
	CIniReader(LPCTSTR szFileName);
	~CIniReader() {};
	int ReadInteger(LPCTSTR szSection, LPCTSTR szKey, int iDefaultValue);
	float ReadFloat(LPCTSTR szSection, LPCTSTR szKey, float fltDefaultValue);
	bool ReadBoolean(LPCTSTR szSection, LPCTSTR szKey, bool bolDefaultValue);
	wstring ReadString(LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szDefaultValue);
private:
	TCHAR m_szFileName[255];
};


class CIniWriter
{
public:
	CIniWriter(LPCTSTR szFileName);
	~CIniWriter() = default;
	void WriteInteger(LPCTSTR szSection, LPCTSTR szKey, int iValue);
	void WriteFloat(LPCTSTR szSection, LPCTSTR szKey, float fltValue);
	void WriteBoolean(LPCTSTR szSection, LPCTSTR szKey, bool bolValue);
	void WriteString(LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szValue);
private:
	TCHAR m_szFileName[255];
};

#endif// INIOPERATION__H__
