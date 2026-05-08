#include "stdafx.h"
#include <shlobj.h>
#include <shellapi.h>
#include <ShlObj_core.h>
#include <process.h>
#include <shellapi.h>
#include <TlHelp32.h>
#include <vector>
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
    if (IsRunAsAdmin())
        return TRUE;

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
                TerminateProcess(hProcess, 0);//关闭进程;
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
    if (cmd == NULL || cmd[0] == L'\0') {
        return S_FALSE;
    }

    auto Trim = [](const std::wstring& text) -> std::wstring {
        const size_t begin = text.find_first_not_of(L" \t\r\n");
        if (begin == std::wstring::npos) {
            return L"";
        }

        const size_t end = text.find_last_not_of(L" \t\r\n");
        return text.substr(begin, end - begin + 1);
        };

    auto ExpandCommand = [](const std::wstring& rawCommand) -> std::wstring {
        DWORD required = ExpandEnvironmentStringsW(rawCommand.c_str(), NULL, 0);
        if (required == 0) {
            return rawCommand;
        }

        std::vector<wchar_t> buffer(required, L'\0');
        DWORD written = ExpandEnvironmentStringsW(rawCommand.c_str(), buffer.data(), required);
        if (written == 0 || written > required) {
            return rawCommand;
        }

        return std::wstring(buffer.data());
        };

    auto ExtractFirstToken = [&](const std::wstring& text) -> std::wstring {
        std::wstring trimmed = Trim(text);
        if (trimmed.empty()) {
            return L"";
        }

        if (trimmed[0] == L'"') {
            const size_t closingQuote = trimmed.find(L'"', 1);
            if (closingQuote != std::wstring::npos) {
                return trimmed.substr(1, closingQuote - 1);
            }
        }

        const size_t separator = trimmed.find_first_of(L" \t");
        if (separator == std::wstring::npos) {
            return trimmed;
        }

        return trimmed.substr(0, separator);
        };

    auto EndsWithNoCase = [](const std::wstring& text, const wchar_t* suffix) -> bool {
        const size_t suffixLen = wcslen(suffix);
        if (text.length() < suffixLen) {
            return false;
        }

        return _wcsicmp(text.c_str() + text.length() - suffixLen, suffix) == 0;
        };

    std::wstring expandedCommand = ExpandCommand(cmd);
    std::wstring executable = ExtractFirstToken(expandedCommand);
    if (executable.empty()) {
        return S_FALSE;
    }

    // MSI packages need to be dispatched via msiexec, while batch files
    // work once the %SystemRoot% / %ComSpec% variables are expanded above.
    if (EndsWithNoCase(executable, L".msi")) {
        expandedCommand = L"msiexec.exe /i " + expandedCommand;
    }

    std::vector<wchar_t> commandLine(expandedCommand.begin(), expandedCommand.end());
    commandLine.push_back(L'\0');

    DWORD ret = S_OK;
    STARTUPINFO si{ sizeof(si) };
    PROCESS_INFORMATION pi{};

    if (hidden) {
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
    }
    if (CreateProcess(NULL,
        commandLine.data(),
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
        APPLOG(Log::LOG_ERROR)("\n---ExecuteProcess failed, last error:%lu---\n", GetLastError());
        return S_FALSE;
    }
}

extern "C"
BOOL DeleteDirectory(const TCHAR* dirPath) 
{
    std::wstring searchPath = std::wstring(dirPath) + L"\\*";
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    do {
        if (wcscmp(findData.cFileName, L".") != 0 && wcscmp(findData.cFileName, L"..") != 0) {
            std::wstring filePath = std::wstring(dirPath) + L"\\" + findData.cFileName;

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (!DeleteDirectory(filePath.c_str())) {
                    FindClose(hFind);
                    return FALSE;
                }
            }
            else {
                SetFileAttributes(filePath.c_str(), FILE_ATTRIBUTE_NORMAL);
                if (!DeleteFile(filePath.c_str())) {
                    FindClose(hFind);
                    return FALSE;
                }
            }
        }
    } while (FindNextFile(hFind, &findData) != 0);

    FindClose(hFind);

    SetFileAttributes(dirPath, FILE_ATTRIBUTE_NORMAL);
    return RemoveDirectory(dirPath) != 0;
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

    Clog::open(pMainFrame->DirUtility().CompanyName().c_str(), (pMainFrame->DirUtility().ProductName() + L"_install.log").c_str(), Log::LOG_TRACE);

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
