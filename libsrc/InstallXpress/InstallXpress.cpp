#include "stdafx.h"
#include <shlobj.h>
#include "MainFrame.h"
#include "Utility/log.h"
#include "InstallXpress.h"

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