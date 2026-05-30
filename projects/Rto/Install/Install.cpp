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
    LANGID uiLang = GetUserDefaultUILanguage();
    if (PRIMARYLANGID(uiLang) == LANG_NEUTRAL) {
        uiLang = GetSystemDefaultUILanguage();
    }
    const bool useChinese = (PRIMARYLANGID(uiLang) == LANG_CHINESE);
    const int skinXmlId = useChinese ? IDR_MAIN_XML : IDR_MAIN_XML_EN;

    TCHAR szSkinXML[16] = { 0 };
    _stprintf_s(szSkinXML, sizeof(szSkinXML) / sizeof(TCHAR) - 1, _T("%d"), skinXmlId);

    InstallXpress_Init_t init_t{
        hInstance,
        hPrevInstance,
        lpCmdLine,
        nCmdShow,

        _T("RTO Setup"),
        szSkinXML,
        IDI_INSTALL,
        IDR_LUA_SCRIPT,
        false,
    };
	return InstallXpress_WinMain(&init_t);
}
