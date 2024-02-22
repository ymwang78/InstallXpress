/*******************************************************
* 文件描叙:电脑基本信息;
* 日期:2015-09-05
*******************************************************/
#ifndef _ComputerInfo_H
#define _ComputerInfo_H

#include <string>

namespace ComputerInfo
{	
	std::wstring GetCurComputerName();	//获取电脑名称;

	std::wstring GetCurUserName();		//获取电脑当前用户;

	std::string  getSystemName();

	long GetCurrentMemoryInfo();
};

#endif