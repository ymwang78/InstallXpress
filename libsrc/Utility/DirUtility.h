/*******************************************************
* �ļ�����:Ŀ¼��Ԫ(����Ŀ¼��ȡ�ȵ�);
* ����:2015-09-05;
*******************************************************/
#ifndef _DIRUTILITY_H
#define _DIRUTILITY_H

#include <string>
using namespace std;

class /*UTILITY_API*/ CDirUtility
{
	std::wstring m_strCompanyName;
	std::wstring m_strProductName;
	std::wstring m_strVersion;
	std::wstring m_strFileDescription;

public:
	CDirUtility();
	~CDirUtility();

public:

	const std::wstring& CompanyName() const { return m_strCompanyName; }

	const std::wstring& ProductName() const { return m_strProductName; }

	const std::wstring& Version() const { return m_strVersion; }

	//��ȡ�û���ʼ/����;
	static std::string GetUserStartProgramPath();

	//��ȡ��ʼ/����;
	static std::string GetStartProgramPath();

	//��ȡ����·��;
	static std::string GetDesktopPath();

	//��ȡ��������Ŀ¼(����������);
	static std::string GetProgramInPath();

	//��ȡ��������;
	static std::string GetProgramName();

	//��ȡ����Ŀ¼;
	static std::string GetProgramPath();

	//ɾ��Ŀ¼;
	static void DeleteAllFile(std::string strFile, bool bRemoveDir = true);

	//��ȡ��ԴĿ¼;
	static std::string GetResDir();

	//��ȡϵͳ�����ļ���;
	static std::wstring GetProgramDir();


	//���ͼƬ�Ƿ���Ч;
	static bool CheckFileInvalid(const string &strFilePath);

	//��ȡͼƬ��չ��;
	static string GetFileSteamExtenName(unsigned char* pImagedata);


	//��ȡappdataĿ¼------vxim;
	wstring GetAppDataDir();

	//��ȡappdataĿ¼------vxim;
	static string GetAppDataChatDir();

	//�����༶Ŀ¼;
	static void CreateMulitDir(const wstring&strdir);

	//��ȡ��ʱĿ¼(appdir׷�ӵ�Ŀ¼);
	std::wstring GetProductTempPath(const std::wstring&strfilename);

	//��ȡ�ļ��汾��Ϣ; < 0 error; >=0 OK
	static int GetFileDescriptionAndProductVersion(LPCWSTR lpstrFilename, 
		wchar_t* lpoutFileDescription, UINT cbFileDescriptionSize,
		wchar_t* lpoutCompanyName, UINT cbCompanySize, 
		wchar_t* lpoutProductName, UINT cbNameSize,
		wchar_t* lpoutProductVersion, UINT cbVerSize);

	//�ж��Ƿ����ļ���;
	static bool IsDir(string szDir);

    static std::wstring GetProgramUpDir();

	//��ȡ������һ��Ŀ¼;
	static std::wstring GetProgramPriovDir();

};

#endif