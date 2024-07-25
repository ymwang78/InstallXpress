#pragma once

#include <wtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

    BOOL IsRunAsAdmin();

    BOOL TryElevate();

    BOOL KillProcess(const wchar_t* szProcessName);

    BOOL FindProcess(const wchar_t* szProcessName);

    DWORD ExecuteProcess(const wchar_t* cmd, bool hidden, int wait_second);

    BOOL DeleteDirectory(const TCHAR* dirPath);

    struct InstallXpress_Init_t
    {
        HINSTANCE hInstance;
        HINSTANCE hPrevInstance;
        LPCTSTR   lpCmdLine;
        int       nCmdShow;

        LPCTSTR   szTitle;
        LPCTSTR   szSkinXML;
        int       nResourceIDIcon;
        int       nResourceIDLua;
    };

    int InstallXpress_WinMain(InstallXpress_Init_t* init_t);


#ifdef __cplusplus
}
#endif