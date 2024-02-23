#pragma once

#include <wtypes.h>

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