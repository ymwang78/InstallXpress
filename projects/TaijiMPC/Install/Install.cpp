// ZhiduInstall.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Install.h"
#include <shlobj.h>
#include "resource.h"
#include "InstallXpress/InstallXpress.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
    TCHAR szSkinXML[16] = { 0 };
    _stprintf_s(szSkinXML, sizeof(szSkinXML) / sizeof(TCHAR) - 1, _T("%d"), IDR_MAIN_XML);

    InstallXpress_Init_t init_t{
        hInstance,
        hPrevInstance,
        lpCmdLine,
        nCmdShow,

        _T("TaijiMPC Setup"),
        szSkinXML,
        IDI_INSTALL,
        IDR_LUA_SCRIPT,
    };
	return InstallXpress_WinMain(&init_t);
}
