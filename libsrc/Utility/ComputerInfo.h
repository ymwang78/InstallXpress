/*******************************************************
* �ļ�����:���Ի�����Ϣ;
* ����:2015-09-05
*******************************************************/
#ifndef _ComputerInfo_H
#define _ComputerInfo_H

#include <string>

namespace ComputerInfo
{	
	std::wstring GetCurComputerName();	//��ȡ��������;

	std::wstring GetCurUserName();		//��ȡ���Ե�ǰ�û�;

	std::string  getSystemName();

	long GetCurrentMemoryInfo();
};

#endif