#include "stdafx.h"
#include "ComputerInfo.h"
#include <WinBase.h>
#include <Psapi.h>
//#include <ntstatus.h>
//#include <iostream>

typedef DWORD(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

namespace ComputerInfo
{
    int GetWindowsVersion(DWORD& dwMajorVersion, DWORD& dwMinorVersion)
    {
		int ret = -1;
        HMODULE hNtDll = LoadLibraryW(L"ntdll.dll");
        if (hNtDll) {
            RtlGetVersionPtr pRtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(GetProcAddress(hNtDll, "RtlGetVersion"));
            if (pRtlGetVersion) {
                RTL_OSVERSIONINFOW osvi;
                ZeroMemory(&osvi, sizeof(osvi));
                osvi.dwOSVersionInfoSize = sizeof(osvi);

                if (pRtlGetVersion(&osvi) == 0) {
					dwMajorVersion = osvi.dwMajorVersion;
					dwMinorVersion = osvi.dwMinorVersion;
					ret = 0;
                }
            }
            FreeLibrary(hNtDll);
        }

        return ret;
    }

	std::wstring GetCurComputerName()
	{
		DWORD size = 0;
		wstring wstr;
		GetComputerName(NULL, &size);  //�õ��������Ƴ���;
		wchar_t *name = new wchar_t[size];
		if (GetComputerName(name, &size))
		{
			wstr = name;
		}
		delete[] name;
		return wstr;
	}

	std::wstring GetCurUserName()
	{
		DWORD size = 0;
		wstring wstr;
		GetUserName(NULL, &size);
		wchar_t *name = new wchar_t[size];
		if (GetUserName(name, &size))
		{
			wstr = name;
		}
		delete[] name;
		return wstr;
	}

	long GetCurrentMemoryInfo()
	{
		DWORD processid = GetCurrentProcessId();
		HANDLE hProcess = NULL;
		PROCESS_MEMORY_COUNTERS pmc;
		long memoryInK = 0;

		// ���m_ProcessID�ľ��
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processid);
		if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
		{
			memoryInK = pmc.WorkingSetSize / 1000;		//��λΪk;			
		}
		CloseHandle(hProcess);
		return memoryInK;
	}

	std::string getSystemName()
	{
		std::string vname = "";
		//���ж��Ƿ�Ϊwin8.1��win10
		typedef void(__stdcall*NTPROC)(DWORD*, DWORD*, DWORD*);
		HINSTANCE hinst = LoadLibrary(_T("ntdll.dll"));
		if (hinst == NULL)
		{
			if (vname.empty()) vname = "Windows 7";
			return vname;
		}

		DWORD dwMajor, dwMinor, dwBuildNumber;
		NTPROC proc = (NTPROC)GetProcAddress(hinst, "RtlGetNtVersionNumbers");
		if (proc == NULL)
		{
			if (vname.empty()) vname = "Windows 7";
			return vname;
		}

		proc(&dwMajor, &dwMinor, &dwBuildNumber);
		if (dwMajor == 6 && dwMinor == 3)	//win 8.1
		{
			vname = "Windows 8.1";
			return vname;
		}
		else if (dwMajor == 10 && dwMinor == 0)	//win 10
		{
			vname = "Windows 10";
			return vname;
		}

		//�ж�win8.1���µİ汾
		SYSTEM_INFO info;                //��SYSTEM_INFO�ṹ�ж�64λAMD������  
		GetSystemInfo(&info);            //����GetSystemInfo�������ṹ  
		OSVERSIONINFOEX os;
		os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		//GetVersionEx((OSVERSIONINFO *)&os)
		if (GetWindowsVersion(os.dwMajorVersion, os.dwMinorVersion) == 0)
		{
			//������ݰ汾��Ϣ�жϲ���ϵͳ����  
			switch (os.dwMajorVersion)
			{ //�ж����汾��  
			case 4:
			{
				switch (os.dwMinorVersion)
				{                //�жϴΰ汾��  
				case 0:
					if (os.dwPlatformId == VER_PLATFORM_WIN32_NT)
						vname = "Windows NT 4.0";  //1996��7�·���  
					else if (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
						vname = "Windows 95";
					break;
				case 10:
					vname = "Windows 98";
					break;
				case 90:
					vname = "Windows Me";
					break;
				}
			}
			break;
			case 5:
			{
				switch (os.dwMinorVersion)
				{//�ٱȽ�dwMinorVersion��ֵ  
				case 0:
					vname = "Windows 2000";
					break;
				default:
					vname = "Windows XP";
					break;
				}
			}
			break;
			case 6:
			{
				switch (os.dwMinorVersion)
				{
				case 0:
					if (os.wProductType == VER_NT_WORKSTATION)
						vname = "Windows Vista";
					else
						vname = "Windows Server 2008";   //�������汾  
					break;
				case 1:
					if (os.wProductType == VER_NT_WORKSTATION)
						vname = "Windows 7";
					else
						vname = "Windows Server 2008 R2";
					break;
				case 2:
					if (os.wProductType == VER_NT_WORKSTATION)
						vname = "Windows 8";
					else
						vname = "Windows Server 2012";
					break;
				}
			}
			break;
			default:
				vname = "Windows Vista";
			}
		}
		if (vname.empty()) vname = "Windows 7";
		return vname;
	}
};