#include "stdafx.h"
#include <shlobj.h>
#include <shellapi.h>
#include <ShlObj_core.h>
#include <process.h>
#include <shellapi.h>
#include <TlHelp32.h>
#include "MainFrame.h"
#include "Utility/log.h"
#include "InstallXpress.h"

extern "C"
BOOL IsRunAsAdmin()
{
    BOOL fIsRunAsAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    if (AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdministratorsGroup)) {
        if (!CheckTokenMembership(NULL, AdministratorsGroup, &fIsRunAsAdmin)) {
            fIsRunAsAdmin = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }
    return fIsRunAsAdmin;
}

extern "C"
BOOL TryElevate()
{
    WCHAR path[MAX_PATH];
    if (GetModuleFileName(NULL, path, ARRAYSIZE(path)) == 0) {
        return false;
    }

    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.lpVerb = L"runas";
    sei.lpFile = path;
    sei.hwnd = NULL;
    sei.nShow = SW_NORMAL;

    if (!ShellExecuteEx(&sei)) {
        DWORD dwError = GetLastError();
        if (dwError == ERROR_CANCELLED) {
            return FALSE;
        }
        //@todo log reason?
        return FALSE;
    }
    else {
        return TRUE;
    }
}


extern "C"
BOOL KillProcess(const wchar_t* szProcessName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return FALSE;
    PROCESSENTRY32 pe32 = { 0 };
    pe32.dwSize = sizeof(pe32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return FALSE;
    }
    do {
        if (_tcsicmp(pe32.szExeFile, szProcessName) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
            if (hProcess) {
                TerminateProcess(hProcess, 0);//¹Ø±Õ½ø³Ì;
                CloseHandle(hProcess);
            }
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return TRUE;
}

extern "C"
BOOL FindProcess(const wchar_t* szProcessName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    PROCESSENTRY32 pe = { sizeof(pe) };
    BOOL bRet = Process32First(hSnapshot, &pe);
    while (bRet)
    {
        if (wcscmp(pe.szExeFile, szProcessName) == 0)
        {
            CloseHandle(hSnapshot);
            return TRUE;
        }
        bRet = Process32Next(hSnapshot, &pe);
    }
    CloseHandle(hSnapshot);
    return FALSE;
}

extern "C"
DWORD ExecuteProcess(const wchar_t* cmd, bool hidden, int wait_second)
{
    DWORD ret = S_OK;
    STARTUPINFO si{ sizeof(si) };
    PROCESS_INFORMATION pi{};

    if (hidden) {
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
    }
    if (CreateProcess(NULL,
        (LPTSTR)cmd,
        NULL,
        NULL,
        FALSE,
        hidden ? CREATE_NO_WINDOW : 0,
        NULL,
        NULL,
        &si,
        &pi)) {
        if (wait_second)
            ret = WaitForSingleObject(pi.hProcess, wait_second < 0 ? INFINITE : wait_second * 1000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return ret;
    }
    else {
        return S_FALSE;
    }
}

extern "C"
int InstallXpress_WinMain(InstallXpress_Init_t* init_t)
{
    ::CoInitialize(NULL);
    ::OleInitialize(NULL);

    TCHAR path[MAX_PATH] = { 0 };
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path);

    CPaintManagerUI::SetInstance(init_t->hInstance);
    CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T(".."));

    CDuiString strCmdLine(init_t->lpCmdLine);

    bool bUpdate = false;
    if (strCmdLine.Find(_T("/D=")) >= 0)
        bUpdate = true;

    CMainFrame* pMainFrame = new CMainFrame(init_t);

    Clog::open((pMainFrame->DirUtility().CompanyName() + L"_" + pMainFrame->DirUtility().ProductName() + L"_install.log").c_str(), Log::ERRORLOG);

    if (pMainFrame) {
        pMainFrame->Create(NULL, init_t->szTitle, UI_CLASSSTYLE_DIALOG, WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 0, 0);
        pMainFrame->ShowWindow();
        CPaintManagerUI::MessageLoop();
    }

    Clog::closeLog();

    ::OleUninitialize();
    ::CoUninitialize();
    return 0;
}