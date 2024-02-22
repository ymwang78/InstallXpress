/*******************************************************
* 文件描叙:常用方法;
* 日期:2015-09-05;
*******************************************************/
#ifndef _TIMEFORMAT_H
#define _TIMEFORMAT_H

#include <string>
using namespace std;

class CGeneralFormat
{
public:
	CGeneralFormat();
	~CGeneralFormat();
public: 
	//将时间格式化为聊天需要的时间格式;
	static std::string FormatChatTime(unsigned int utime);

	//将时间格式化为聊天需要的时间格式;
	static std::string TimeToString(unsigned int utime);
	
	//获取当前时间戳;
	static long long GetCurTime();

	//生成GUID;
	static std::string GenerateGuid();

	//Md5;
	static std::string BuildMd5(string strtext);

	//文件Md5;
	static std::string BuildFileMd5(const wchar_t* filepath);

	//大数字转换为字符串;
	static std::wstring BigNumToString(UINT64 lNum);
};

#endif