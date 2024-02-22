#pragma once

#include <string>
using namespace std;

struct stSharePiuInfo
{
	unsigned int iid;
	HWND hwnd;
	string strsession;
	unsigned int npid;
	int ndatalen;
	string strdata;
	stSharePiuInfo() :iid(0), hwnd(NULL), npid(0), strsession(""), ndatalen(0), strdata("") {};
};

struct stLaiWanAc
{
	long long upiucion;
	long long upiuscore;
	unsigned int iid;
	stLaiWanAc() :upiuscore(0), upiucion(0), iid(0) {};
};

class CShareMemory
{
public:
	CShareMemory(int iShareMemSize, wchar_t* pShareMemName);
	~CShareMemory();


	void* GetAddr();												//获取共享内存地址;
	void* GetMappingData();											//获取内存数据;
	void  CloseMapping();											//关闭共享内存;

public:
	bool Lock(bool bclose);
	bool UnLock();
	bool WriterData(stSharePiuInfo &stdata);
	bool ReadData(stSharePiuInfo &stdata);
	bool WriterData(stLaiWanAc &stdata);
	bool ReadData(stLaiWanAc &stdata);
	bool WriterData(HWND hwnd);
	HWND ReadData();

private:
	int			m_iShareMemSize;
	wstring		m_strShareMemName;

	HANDLE      m_hMutex;
	HANDLE		m_hMapFile;
	LPVOID		m_lpMapAddress;										//共享内存地址;
};

