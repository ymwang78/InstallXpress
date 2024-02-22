/*******************************************************
* 文件描叙:目录单元(程序目录获取等等);
* 日期:2015-09-05;
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

	//获取用户开始/程序;
	static std::string GetUserStartProgramPath();

	//获取开始/程序;
	static std::string GetStartProgramPath();

	//获取桌面路径;
	static std::string GetDesktopPath();

	//获取程序所在目录(不含程序名);
	static std::string GetProgramInPath();

	//获取程序名称;
	static std::string GetProgramName();

	//获取程序目录;
	static std::string GetProgramPath();

	//删除目录;
	static void DeleteAllFile(std::string strFile, bool bRemoveDir = true);

	//获取资源目录;
	static std::string GetResDir();

	//获取系统程序文件夹;
	static std::wstring GetProgramDir();


	//检测图片是否有效;
	static bool CheckFileInvalid(const string &strFilePath);

	//获取图片扩展名;
	static string GetFileSteamExtenName(unsigned char* pImagedata);


	//获取appdata目录------vxim;
	wstring GetAppDataDir();

	//获取appdata目录------vxim;
	static string GetAppDataChatDir();

	//创建多级目录;
	static void CreateMulitDir(const wstring&strdir);

	//获取临时目录(appdir追加的目录);
	std::wstring GetProductTempPath(const std::wstring&strfilename);

	//获取文件版本信息; < 0 error; >=0 OK
	static int GetFileDescriptionAndProductVersion(LPCWSTR lpstrFilename, 
		wchar_t* lpoutFileDescription, UINT cbFileDescriptionSize,
		wchar_t* lpoutCompanyName, UINT cbCompanySize, 
		wchar_t* lpoutProductName, UINT cbNameSize,
		wchar_t* lpoutProductVersion, UINT cbVerSize);

	//判断是否是文件夹;
	static bool IsDir(string szDir);

    static std::wstring GetProgramUpDir();

	//获取程序上一级目录;
	static std::wstring GetProgramPriovDir();

};

#endif