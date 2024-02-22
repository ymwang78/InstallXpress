#include "stdafx.h"
#include "ChineseToPy.h"
#include "..\resource.h"

///////////////////////////////////////////////////////////////
//
//
CSearchData::CSearchData()
	: m_utype(NOSEARCH_TYPE)
	, m_strid(_T(""))
	, m_strname(_T(""))
	, m_strnameFirstletter(_T(""))
	, m_strnameAllAlphabet(_T(""))
	, m_strremark(_T(""))
	, m_strremarkFirstletter(_T(""))
	, m_strremarkAllAlphabet(_T(""))
	, m_strsiteid(_T(""))
{
	m_upid = 0;
	m_uid = 0;
}

CSearchData::~CSearchData()
{
}

void CSearchData::SetData(unsigned short utype, unsigned int uid, unsigned int upid, std::wstring strname, std::wstring strremark, std::wstring strsiteid)
{
	m_utype = utype;
	m_uid = uid;
	m_upid = upid;
	m_strid = UIntToWstr(m_upid);
	m_strsiteid = strsiteid;

	string strpy = "";
	string strallpy = "";
	m_strname = strname;
	m_strremark = strremark;

	if (!m_strname.empty())
	{
		string strnickname = WtoS(strname);
		CChineseToPy::ChineseToPy(strpy, strallpy, strnickname.c_str(), PY_UPPERCASE | PY_FIRSTCHAR);
		m_strnameFirstletter = StoW(strpy);
		m_strnameAllAlphabet = StoW(strallpy);
	}
	if (!strremark.empty())
	{
		strpy.clear();
		strallpy.clear();
		string strremarktext = WtoS(strremark);
		CChineseToPy::ChineseToPy(strpy, strallpy, strremarktext.c_str(), PY_UPPERCASE | PY_FIRSTCHAR);
		m_strremarkFirstletter = StoW(strpy);
		m_strremarkAllAlphabet = StoW(strallpy);
	}
}

//检索数据匹配;
bool CSearchData::MatchingContent(std::wstring strmatetext)
{
	if (strmatetext.empty())
		return false;

	if (!m_strid.empty() && m_strid.find(strmatetext) != std::wstring::npos && m_strid != _T("0"))
	{ //ID匹配;
		return true;
	}
	string strpy = "";
	string strallpy = "";
	string smatetext = WtoS(strmatetext);

	CChineseToPy::ChineseToPy(strpy, strallpy, smatetext.c_str(), PY_UPPERCASE | PY_FIRSTCHAR);
	std::wstring ustrfirstpy = StoW(strpy);
	std::wstring ustrallpy = StoW(strallpy);


	if (!m_strname.empty())
	{
		if (m_strname.find(strmatetext) != std::wstring::npos)
			return true;
		if (m_strnameFirstletter.find(ustrfirstpy) != std::wstring::npos)
			return true;
		if (m_strnameAllAlphabet.find(ustrallpy) != std::wstring::npos)
			return true;
	}
	if (!m_strremark.empty())
	{
		if (m_strremark.find(strmatetext) != std::wstring::npos)
			return true;
		if (m_strremarkFirstletter.find(ustrfirstpy) != std::wstring::npos)
			return true;
		if (m_strremarkAllAlphabet.find(ustrallpy) != std::wstring::npos)
			return true;
	}
	return false;
}

static HGLOBAL s_hGlobale = NULL;
// 全局变量;
LPSTR g_pszRes = NULL;
LPSTR g_pszResEnd = NULL;

CChineseToPy::CChineseToPy()
{

}


CChineseToPy::~CChineseToPy()
{
}

void CChineseToPy::InitPyDictionary(HMODULE hModule,DWORD dResId)
{	
	HRSRC hRsrc;
	// 载入拼音数据;
	hRsrc = FindResource(hModule, MAKEINTRESOURCE(dResId), L"PY");
	if (hRsrc)
	{
		s_hGlobale = LoadResource(hModule, hRsrc);
		if (s_hGlobale)
		{
			g_pszRes = (LPSTR)LockResource(s_hGlobale);
			g_pszResEnd = g_pszRes + SizeofResource(hModule, hRsrc) - 16;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// 中文字符串转换为拼音;
// 参数:;
//       pszCC    以空字符结为的中文字符串;
//       pszPY    转换后的拼音字符串，如为 NULL，只返回拼音字符串的长度，否则请确保有足够大的空间;
//       dwFlags  指明操作标志，详细请看前面的宏定义;
// 返回: 返回拼音字符串的长度;
int CChineseToPy::ChineseToPy(std::string &strpy, std::string &strallpy, LPCSTR pszCC, DWORD dwFlags)
{
	unsigned char * p;		// 检索拼音数据可移动指针;
	unsigned char * pPY;	// 输出拼音可移动指针;
	unsigned char * pCC;	// 输入中文字符可移动指针;
	unsigned char * pCCEnd;	// 输入中文字符结束指针;
	char pszPY[MAX_PATH] = { 0 };

	// 循环检索中文字符串;
	for (pPY = (unsigned char*)pszPY, pCC = (unsigned char *)pszCC, pCCEnd = (unsigned char *)pszCC + lstrlenA(pszCC); pCC < pCCEnd; pCC++)
	{
		// 如果可能是中文字符;
		if ((*pCC & 0x80) && (pCC < pCCEnd - 1) && (*(pCC + 1) & 0x80))
		{
			// 循环检索拼音数据;
			for (p = (unsigned char*)g_pszRes; p < (unsigned char*)g_pszResEnd; p++)
			{
				// 如果找到;
				if (*((WORD *)p) == *((WORD *)pCC))
					break;
			}

			// 如果检索到拼音;
			if (p < (unsigned char*)g_pszResEnd)
			{
				// 如果要加前导空格;
				if ((dwFlags & PY_LEADSPACE) && (pCC != (unsigned char*)pszCC))
				{
					if (pszPY)
						*pPY = ' ';
					pPY++;
				}

				// 提取首字母;
				if (dwFlags & PY_FIRSTCHAR)
				{
					string strpinyin = (LPSTR)p + 2;
					if (!strpinyin.empty())
					{
						strpy.push_back(strpinyin.at(0));
					}
				}

				// 复制;
				if (pszPY)
					lstrcpyA((LPSTR)(pPY), (LPSTR)p + 2);

				// 设置 pCC 和 pPY;
				pPY += lstrlenA((LPSTR)p + 2);
			}
			// 如果没有检索到拼音 (比如全角标点符号);
			else
			{
				if (pszPY)
				{
					*((WORD *)pPY) = *((WORD *)pCC);
				}

				// 提取首字母(符号直接赋值);
				if (dwFlags & PY_FIRSTCHAR)
				{
					string strpinyin = (LPSTR)(pCC);
					if (strpinyin.length()>=2)
					{
						strpy.append(strpinyin.substr(0,2));
					}
				}
				pPY += 2;				
			}
			pCC++;
		}
		// 如果不可能是中文字符;
		else
		{
			if (pszPY)
				*pPY = *pCC;

			// 提取首字母(非中文);
			if (dwFlags & PY_FIRSTCHAR)
			{
				strpy.push_back(*pCC);
			}

			pPY++;
		}
	}

	// 置结尾标志字符;
	if (pszPY)
	{
		*pPY = '\0';

		// 如果拼音全部大写;
		if (dwFlags & PY_UPPERCASE)
			CharUpperA(pszPY);
		// 如果拼音全部小写;
		else if (dwFlags & PY_LOWERCASE)
			CharLowerA(pszPY);
	}

	// 如果拼音全部大写;
	if (dwFlags & PY_UPPERCASE)
		CharUpperA((LPSTR)strpy.c_str());
	// 如果拼音全部小写;
	else if (dwFlags & PY_LOWERCASE)
		CharLowerA((LPSTR)strpy.c_str());

	strallpy = pszPY;	//全拼;

	return 0;
}

void CChineseToPy::UnInitPyDictionary()
{
	if (s_hGlobale)
		FreeResource(s_hGlobale);
}