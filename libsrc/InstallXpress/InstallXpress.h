#pragma once

#include <wtypes.h>

// Conventional resource ID layout for InstallXpress projects.
// Each project defines these in its own resource.h; these values document
// the expected slot assignments so Lua scripts can reference named constants.
#ifdef __cplusplus
namespace InstallXpress {
    constexpr int RESID_ICON        = 107;  // IDI_INSTALL
    constexpr int RESID_SKIN_XML    = 131;  // IDR_MAIN_XML
    constexpr int RESID_LICENSE     = 133;  // IDR_REGCONTENT1
    constexpr int RESID_SOFT1       = 134;  // IDR_INSTALLSOFT1 (primary package)
    constexpr int RESID_LUA_SCRIPT  = 135;  // IDR_LUA_SCRIPT
    constexpr int RESID_BACKGROUND  = 138;  // IDB_RES_BACKGROUND
    constexpr int RESID_SOFT2       = 139;  // IDR_INSTALLSOFT2
    constexpr int RESID_SOFT3       = 140;  // IDR_INSTALLSOFT3
}
#endif // __cplusplus

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