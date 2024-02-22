/*******************************************************
* �ļ�����:����תƴ��;
* ����:2015-11-26;
*******************************************************/
#ifndef _CHINESETOPY_H
#define _CHINESETOPY_H

#define PY_LEADSPACE	0x00000001	// ÿ��ƴ����ǰ���ո�;
#define PY_LOWERCASE	0x00000002	// ���Сдƴ���ַ���;
#define PY_UPPERCASE	0x00000004	// �����дƴ���ַ���;
#define PY_FIRSTCHAR	0x00000008  // �����������ĸ�ַ���;

///////////////////////////////////////////////////////////
class CSearchData
{
public:
	enum sDataType
	{
		FRIEND_TYPE = 0,
		ROOM_TYPE,
		SITE_TYPE,
		NOSEARCH_TYPE,
	};

public:
	CSearchData();
	~CSearchData();

public:
	//��ʼ����������;
	void SetData(unsigned short utype,unsigned int uid, unsigned int upid, std::wstring strname, std::wstring strremark, std::wstring strsiteid);
	//��������ƥ��;
	bool MatchingContent(std::wstring strmatetext);

public:
	unsigned short m_utype;
	unsigned int   m_uid;
	unsigned int   m_upid;
	std::wstring m_strsiteid;
	std::wstring m_strid;
	std::wstring m_strname;
	std::wstring m_strnameFirstletter;
	std::wstring m_strnameAllAlphabet;

	std::wstring m_strremark;
	std::wstring m_strremarkFirstletter;
	std::wstring m_strremarkAllAlphabet;
};


class CChineseToPy
{
public:
	CChineseToPy();
	~CChineseToPy();

public:
	static void InitPyDictionary(HMODULE hModule, DWORD dResId);
	static int  ChineseToPy(std::string &strpy, std::string &strallpy, LPCSTR pszCC, DWORD dwFlags);
	static void UnInitPyDictionary();
};

#endif //_CHINESETOPY_H