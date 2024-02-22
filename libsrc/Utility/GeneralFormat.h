/*******************************************************
* �ļ�����:���÷���;
* ����:2015-09-05;
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
	//��ʱ���ʽ��Ϊ������Ҫ��ʱ���ʽ;
	static std::string FormatChatTime(unsigned int utime);

	//��ʱ���ʽ��Ϊ������Ҫ��ʱ���ʽ;
	static std::string TimeToString(unsigned int utime);
	
	//��ȡ��ǰʱ���;
	static long long GetCurTime();

	//����GUID;
	static std::string GenerateGuid();

	//Md5;
	static std::string BuildMd5(string strtext);

	//�ļ�Md5;
	static std::string BuildFileMd5(const wchar_t* filepath);

	//������ת��Ϊ�ַ���;
	static std::wstring BigNumToString(UINT64 lNum);
};

#endif