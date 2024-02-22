// ZhiduInstall.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Install.h"
#include <shlobj.h>
#include "MainFrame.h"
#include "Utility/log.h"

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

	CDuiString strCmdLine(lpCmdLine);

	bool bUpdate = false;
	if (strCmdLine.Find(_T("/D=")) >= 0) 
		bUpdate = true;

	CMainFrame* pMainFrame = new CMainFrame(bUpdate);

    Clog::open((pMainFrame->DirUtility().CompanyName() + L"_" + pMainFrame->DirUtility().ProductName() + L"_install.log").c_str(), Log::ERRORLOG);

	if (pMainFrame)
	{
		pMainFrame->Create(NULL, _T("安装程序"), UI_CLASSSTYLE_DIALOG, WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 0, 0);
		pMainFrame->ShowWindow();
		CPaintManagerUI::MessageLoop();
	}

	Clog::closeLog();

	::OleUninitialize();
	::CoUninitialize();
	return 0;
}