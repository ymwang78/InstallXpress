// UnInstall.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "UnInstall.h"
#include <shlobj.h>
#include "MainFrame.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	::CoInitialize(NULL);
	::OleInitialize(NULL);

	TCHAR path[MAX_PATH] = { 0 };
	SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path);

	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T(".."));

	CUninstallMainFrame* pMainFrame = new CUninstallMainFrame();
	if (pMainFrame)
	{
		pMainFrame->Create(NULL, _T("泰极MPC卸载"), UI_CLASSSTYLE_DIALOG, WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 0, 0);
		pMainFrame->ShowWindow();
		CPaintManagerUI::MessageLoop();
	}

	::OleUninitialize();
	::CoUninitialize();

    return (int)0;
}
