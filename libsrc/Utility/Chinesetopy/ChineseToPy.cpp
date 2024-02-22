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

//��������ƥ��;
bool CSearchData::MatchingContent(std::wstring strmatetext)
{
	if (strmatetext.empty())
		return false;

	if (!m_strid.empty() && m_strid.find(strmatetext) != std::wstring::npos && m_strid != _T("0"))
	{ //IDƥ��;
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
// ȫ�ֱ���;
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
	// ����ƴ������;
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
// �����ַ���ת��Ϊƴ��;
// ����:;
//       pszCC    �Կ��ַ���Ϊ�������ַ���;
//       pszPY    ת�����ƴ���ַ�������Ϊ NULL��ֻ����ƴ���ַ����ĳ��ȣ�������ȷ�����㹻��Ŀռ�;
//       dwFlags  ָ��������־����ϸ�뿴ǰ��ĺ궨��;
// ����: ����ƴ���ַ����ĳ���;
int CChineseToPy::ChineseToPy(std::string &strpy, std::string &strallpy, LPCSTR pszCC, DWORD dwFlags)
{
	unsigned char * p;		// ����ƴ�����ݿ��ƶ�ָ��;
	unsigned char * pPY;	// ���ƴ�����ƶ�ָ��;
	unsigned char * pCC;	// ���������ַ����ƶ�ָ��;
	unsigned char * pCCEnd;	// ���������ַ�����ָ��;
	char pszPY[MAX_PATH] = { 0 };

	// ѭ�����������ַ���;
	for (pPY = (unsigned char*)pszPY, pCC = (unsigned char *)pszCC, pCCEnd = (unsigned char *)pszCC + lstrlenA(pszCC); pCC < pCCEnd; pCC++)
	{
		// ��������������ַ�;
		if ((*pCC & 0x80) && (pCC < pCCEnd - 1) && (*(pCC + 1) & 0x80))
		{
			// ѭ������ƴ������;
			for (p = (unsigned char*)g_pszRes; p < (unsigned char*)g_pszResEnd; p++)
			{
				// ����ҵ�;
				if (*((WORD *)p) == *((WORD *)pCC))
					break;
			}

			// ���������ƴ��;
			if (p < (unsigned char*)g_pszResEnd)
			{
				// ���Ҫ��ǰ���ո�;
				if ((dwFlags & PY_LEADSPACE) && (pCC != (unsigned char*)pszCC))
				{
					if (pszPY)
						*pPY = ' ';
					pPY++;
				}

				// ��ȡ����ĸ;
				if (dwFlags & PY_FIRSTCHAR)
				{
					string strpinyin = (LPSTR)p + 2;
					if (!strpinyin.empty())
					{
						strpy.push_back(strpinyin.at(0));
					}
				}

				// ����;
				if (pszPY)
					lstrcpyA((LPSTR)(pPY), (LPSTR)p + 2);

				// ���� pCC �� pPY;
				pPY += lstrlenA((LPSTR)p + 2);
			}
			// ���û�м�����ƴ�� (����ȫ�Ǳ�����);
			else
			{
				if (pszPY)
				{
					*((WORD *)pPY) = *((WORD *)pCC);
				}

				// ��ȡ����ĸ(����ֱ�Ӹ�ֵ);
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
		// ����������������ַ�;
		else
		{
			if (pszPY)
				*pPY = *pCC;

			// ��ȡ����ĸ(������);
			if (dwFlags & PY_FIRSTCHAR)
			{
				strpy.push_back(*pCC);
			}

			pPY++;
		}
	}

	// �ý�β��־�ַ�;
	if (pszPY)
	{
		*pPY = '\0';

		// ���ƴ��ȫ����д;
		if (dwFlags & PY_UPPERCASE)
			CharUpperA(pszPY);
		// ���ƴ��ȫ��Сд;
		else if (dwFlags & PY_LOWERCASE)
			CharLowerA(pszPY);
	}

	// ���ƴ��ȫ����д;
	if (dwFlags & PY_UPPERCASE)
		CharUpperA((LPSTR)strpy.c_str());
	// ���ƴ��ȫ��Сд;
	else if (dwFlags & PY_LOWERCASE)
		CharLowerA((LPSTR)strpy.c_str());

	strallpy = pszPY;	//ȫƴ;

	return 0;
}

void CChineseToPy::UnInitPyDictionary()
{
	if (s_hGlobale)
		FreeResource(s_hGlobale);
}