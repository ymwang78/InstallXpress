#include "stdafx.h"
#include "ShareMemory.h"


CShareMemory::CShareMemory(int iShareMemSize, wchar_t* pShareMemName)
	: m_iShareMemSize(iShareMemSize)
	, m_strShareMemName(pShareMemName)
	, m_lpMapAddress(NULL)
	, m_hMapFile(NULL)
	, m_hMutex(NULL)
{
}


CShareMemory::~CShareMemory()
{
	CloseMapping();
}

//¹Ø±Õ¹²ÏíÄÚ´æ;
void CShareMemory::CloseMapping()
{
	if (m_lpMapAddress)
	{
		UnmapViewOfFile(m_lpMapAddress);
		m_lpMapAddress = NULL;
	}

	if (m_hMapFile)
	{
		CloseHandle(m_hMapFile);
		m_hMapFile = NULL;
	}

	if (m_hMutex)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
}

bool CShareMemory::Lock(bool bclose)
{
	wstring strmutextname = m_strShareMemName;
	strmutextname.append(_T("_mtx"));
	m_hMutex = CreateMutex(NULL, FALSE, strmutextname.c_str());
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
		return false;
	}
	if (bclose) {
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
	return true;
}

bool CShareMemory::UnLock()
{
	if (m_hMutex)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
	return true;
}

void* CShareMemory::GetAddr()
{
	if (NULL == m_lpMapAddress)
	{
		if (NULL == m_hMapFile)
		{
			m_hMapFile = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, m_iShareMemSize, m_strShareMemName.c_str());
		}

		if (m_hMapFile)
		{
			m_lpMapAddress = MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		}
	}
	return m_lpMapAddress;
}

void* CShareMemory::GetMappingData()
{
	if (NULL == m_lpMapAddress)
	{
		if (m_hMapFile == NULL)
		{
			m_hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, m_strShareMemName.c_str());
		}
		if (m_hMapFile)
		{
			m_lpMapAddress = MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		}
	}
	return m_lpMapAddress;
}

bool CShareMemory::WriterData(stSharePiuInfo &stdata)
{
	int wlen = 0;
	char* sharemem = (char*)GetAddr();
	if (sharemem == NULL)
		return false;

	Lock(false);

	memcpy(sharemem + wlen, &stdata.iid, 4);
	wlen += 4;
	memcpy(sharemem + wlen, &stdata.hwnd, sizeof(HWND));
	wlen += sizeof(HWND);
	memcpy(sharemem + wlen, stdata.strsession.c_str(), stdata.strsession.length());
	wlen += 32;
	memcpy(sharemem + wlen, &stdata.npid, 4);
	wlen += 4;
	int datalen = stdata.ndatalen;

	if ((datalen + wlen) >= m_iShareMemSize) 
		datalen = 0;
	memcpy(sharemem + wlen, &datalen, 4);

	wlen += 4;
	if ((wlen + stdata.ndatalen) < m_iShareMemSize)
	{
		memcpy(sharemem + wlen, stdata.strdata.c_str(), stdata.strdata.length());
		wlen += stdata.strdata.length();
	}
	UnLock();
	return true;
}

bool CShareMemory::ReadData(stSharePiuInfo &stdata)
{
	if (Lock(true) == false)
		return false;

	char* pMapmem = (char*)GetMappingData();
	if (pMapmem == NULL)
		return false;
	char csession[33] = { 0 };
	int nreadlen = 0;
	memcpy(&stdata.iid, pMapmem + nreadlen, 4);
	nreadlen += 4;
	memcpy(&stdata.hwnd, pMapmem + nreadlen, sizeof(HWND));
	nreadlen += sizeof(HWND);
	memcpy(csession, pMapmem + nreadlen, 32);
	stdata.strsession.append(csession);
	nreadlen += 32;
	memcpy(&stdata.npid, pMapmem + nreadlen, 4);
	nreadlen += 4;
	memcpy(&stdata.ndatalen, pMapmem + nreadlen, 4);
	nreadlen += 4;
	if (stdata.ndatalen > 0)
	{
		char* pData = new char[stdata.ndatalen + 1];
		memset(pData, 0, stdata.ndatalen + 1);
		memcpy(pData, pMapmem + nreadlen, stdata.ndatalen);
		string strdata(pData);
		stdata.strdata = strdata;
		if (pData) { delete[] pData; pData = NULL; }
	}
	return true;
}

bool CShareMemory::WriterData(stLaiWanAc &stdata)
{
	char* sharemem = (char*)GetAddr();
	if (sharemem == NULL)
		return false;
	Lock(false);
	int wlen = 0;
	memcpy(sharemem, &stdata.upiucion, sizeof(long long));
	wlen += sizeof(long long);
	memcpy(sharemem + wlen, &stdata.upiuscore, sizeof(long long));
	wlen += sizeof(long long);
	memcpy(sharemem + wlen, &stdata.iid, sizeof(unsigned int));
	UnLock();
	return true;
}

bool CShareMemory::ReadData(stLaiWanAc &stdata)
{
	if (Lock(true) == false)
		return false;

	char* pMapmem = (char*)GetMappingData();
	if (pMapmem == NULL)
		return false;

	int nreadlen = 0;
	memcpy(&stdata.upiucion, pMapmem, sizeof(long long));
	nreadlen += sizeof(long long);
	memcpy(&stdata.upiuscore, pMapmem + nreadlen, sizeof(long long));
	nreadlen += sizeof(long long);
	memcpy(&stdata.iid, pMapmem + nreadlen, sizeof(unsigned int));

	return true;
}

bool CShareMemory::WriterData(HWND hwnd)
{
	char* sharemem = (char*)GetAddr();
	if (sharemem == NULL)
		return false;
	Lock(false);
	memcpy(sharemem, &hwnd, sizeof(HWND));
	UnLock();
	return true;
}

HWND CShareMemory::ReadData()
{
	if (Lock(true) == false)
		return false;

	char* pMapmem = (char*)GetMappingData();
	if (pMapmem == NULL)
		return NULL;

	HWND hwnd = NULL;
	memcpy(&hwnd, pMapmem, sizeof(HWND));
	return hwnd;
}