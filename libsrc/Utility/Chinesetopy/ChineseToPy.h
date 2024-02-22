/*******************************************************
* 文件描叙:中文转拼音;
* 日期:2015-11-26;
*******************************************************/
#ifndef _CHINESETOPY_H
#define _CHINESETOPY_H

#define PY_LEADSPACE	0x00000001	// 每个拼音加前导空格;
#define PY_LOWERCASE	0x00000002	// 输出小写拼音字符串;
#define PY_UPPERCASE	0x00000004	// 输出大写拼音字符串;
#define PY_FIRSTCHAR	0x00000008  // 输出汉字首字母字符串;

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
	//初始化检索数据;
	void SetData(unsigned short utype,unsigned int uid, unsigned int upid, std::wstring strname, std::wstring strremark, std::wstring strsiteid);
	//检索数据匹配;
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