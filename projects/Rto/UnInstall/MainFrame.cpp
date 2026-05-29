#include "stdafx.h"
#include "MainFrame.h"
#include "resource.h"
#include <ShlObj.h>
#include <process.h>
#include <shellapi.h>
#include <Shlwapi.h>
#include <TlHelp32.h>
#include "Utility/Utility.h"
#include <Utility/DirUtility.h>
#include <Utility/LuaExtention.h>
#include <Utility/ResourceHandler.h>

#pragma comment(lib, "liblua.lib")

#define  WMPROGRESS_TIMER WM_USER+1000
#define  WMPROGRESSFINISH_TIMER WM_USER+1001
#define  WM_INSTALLPROGRES_MSG WM_USER+2000

#define  PORGRESSSHOWSEPLEN 50	  //进度条显示定时器步长;
#define  PORGRESSHIDESEPLEN 20	  //进度条隐藏定时器步长;

#define  MOVESIZE 20
#define  LEFTOFFSET 40            //进度条左侧距离;
#define  BUTTONMINSIZE 200		  //按钮最小值;

CMainFrame::CMainFrame()
	:m_bUninstall(false)
{
	m_pUnInstallprogress = NULL;
	m_pStarUnInstallbtn = NULL;
	m_pUnFinishbtn = NULL;
	m_strname = _T("");
	m_strplid = _T("zdey");
}


CMainFrame::~CMainFrame()
{
	PostQuitMessage(0);
}

void CMainFrame::InitWindow()
{
	CenterWindow();
}

LPCTSTR CMainFrame::GetWindowClassName(void) const
{
	return _T("ZhiduMainFrame");
}

CDuiString CMainFrame::GetSkinFile()
{
	TCHAR szBuf[MAX_PATH] = { 0 };
	_stprintf_s(szBuf, MAX_PATH - 1, _T("%d"), IDR_MAIN_XML);
	return szBuf;
}

CDuiString CMainFrame::GetSkinFolder()
{
	return _T("");
}

UILIB_RESOURCETYPE CMainFrame::GetResourceType() const
{
	return UILIB_RESOURCE;
}

void CMainFrame::OnFinalMessage(HWND hWnd)
{
	delete this;
}

ResourceHandler* CMainFrame::LoadResourceFile(UINT uId, LPCTSTR lpType)
{
    auto iter = m_resHandlerMap.find(uId);
    if (iter != m_resHandlerMap.end())
        return iter->second;
    ResourceHandler* handler = new ResourceHandler(m_PaintManager.GetResourceDll(), MAKEINTRESOURCE(uId), lpType);
    if (handler->GetData() == NULL) {
        delete handler;
        return NULL;
    }
    m_resHandlerMap.insert(std::make_pair(uId, handler));
    return handler;
}

void CMainFrame::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, DUI_MSGTYPE_WINDOWINIT) == 0)
	{
		string file = CDirUtility::GetProgramPath();
		m_filepath = CTypeConvertUtil::StringToWstring(file);

		m_pUnInstallprogress = static_cast<CProgressUI*>(m_PaintManager.FindControl(_T("uninstallprogress")));
		m_pStarUnInstallbtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("staruninstallbtn")));
		m_pUnFinishbtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("uninstallfinishbtn")));
		m_pCloseBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("closebtn")));
        m_luaPtr = new InstallLua(&m_PaintManager, UnicodeToUtf8(CDirUtility().Version()));
#ifdef DEBUG
        m_luaPtr->load_file("UnInstall.lua");
#else
        ResourceHandler* luaScript = LoadResourceFile(IDR_LUA_SCRIPT, _T("LUA_SCRIPT"));
        m_luaPtr->load_string((const char*)luaScript->GetData());
#endif
        m_luaPtr->Initialize();
	}
	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_CLICK) == 0)
	{
		if (_tcsicmp(msg.pSender->GetName(), _T("staruninstallbtn")) == 0)
		{
			if (m_pStarUnInstallbtn) m_pStarUnInstallbtn->SetVisible(false);
			if (m_pUnInstallprogress) m_pUnInstallprogress->SetVisible(true);

			if (m_pCloseBtn) m_pCloseBtn->SetEnabled(false);
			SetTimer(this->GetHWND(), WMPROGRESS_TIMER, PORGRESSSHOWSEPLEN, 0);

            m_luaPtr->PreSetup();

		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("uninstallfinishbtn")) == 0)
		{
			ClearUnInstall();			
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("closebtn")) == 0)
		{
			if (m_bUninstall)
				ClearUnInstall();
			else
				PostMessage(WM_CLOSE, 0, 0);
		}
	}
}

LRESULT CMainFrame::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_TIMER)
	{
		if (wParam == WMPROGRESS_TIMER)
		{
			if (NULL == m_pUnInstallprogress)
			{
				::KillTimer(this->GetHWND(), WMPROGRESS_TIMER);
			}
			else
			{
				RECT rc = m_pUnInstallprogress->GetPos();
				if (rc.left - MOVESIZE < LEFTOFFSET)
				{
					m_pUnInstallprogress->SetValue(15);
					::KillTimer(this->GetHWND(), WMPROGRESS_TIMER);
					m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CMainFrame::UnInstallThread, this, 0, NULL);
					return 0;
				}
				rc.left -= MOVESIZE;
				rc.right += MOVESIZE;
				m_pUnInstallprogress->SetPos(rc);
			}
		}
		else if (WMPROGRESSFINISH_TIMER == wParam)
		{
			if (NULL == m_pUnInstallprogress)
			{
				::KillTimer(this->GetHWND(), WMPROGRESSFINISH_TIMER);
				return 0;
			}
			RECT rc = m_pUnInstallprogress->GetPos();
			if (rc.right - rc.left <= BUTTONMINSIZE)
			{
				::KillTimer(this->GetHWND(), WMPROGRESSFINISH_TIMER);
				m_pUnInstallprogress->SetVisible(false);
				m_pUnFinishbtn->SetVisible(true);
				if (m_pCloseBtn) m_pCloseBtn->SetEnabled(true);
				m_bUninstall = true;
				return 0;
			}
			rc.left += MOVESIZE;
			rc.right -= MOVESIZE;
			m_pUnInstallprogress->SetPos(rc);
		}
	}
	return WindowImplBase::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CMainFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (WM_INSTALLPROGRES_MSG == uMsg)
	{
		if (m_pUnInstallprogress)
		{
			DUITRACE(_T("- UnInstallprogress: %lu \n"), lParam);
			m_pUnInstallprogress->SetValue(lParam);
			CDuiString str;
			str.Format(_T("已卸载%d%%"), lParam);
			m_pUnInstallprogress->SetText(str.GetData());
		}
	}
	return WindowImplBase::HandleCustomMessage(uMsg, wParam, lParam, bHandled);
}

unsigned int _stdcall CMainFrame::UnInstallThread(void* param)
{
	CMainFrame* pThis = (CMainFrame*)param;
	if (NULL != pThis)
		pThis->UnInstall();
	return 0;
}

void CMainFrame::ClearUnInstall()
{
	std::wstring strsite = m_strname;
	std::wstring strdesp = m_strname;
	std::wstring strpath = _T("");
	std::wstring strProdname = m_strname;
	
	CRegOperation reg;
    std::wstring strkey = L"Software" L"\\" COMPANYNAME;

	reg.DeleteKey(HKEY_CURRENT_USER, strkey.c_str(), PRODUCTNAME);

	char syscmd[MAX_PATH] = { 0 };
	sprintf_s(syscmd, "/C ping 127.0.0.1 -n 5 > nul & rmdir /s /Q \"%s\" ", m_strUnInstallDir.c_str());
	ShellExecuteA(NULL, NULL, "cmd.exe", syscmd, NULL, SW_HIDE);
	PostMessage(WM_CLOSE, 0, 0);
}

void CMainFrame::UnInstall()
{
	m_pUnInstallprogress->SetText(_T("已卸载1%..."));
	m_strUnInstallDir = CDirUtility::GetProgramInPath();
	//int lastpos = m_strUnInstallDir.find_last_of('\\');
	//m_strUnInstallDir = m_strUnInstallDir.substr(0, lastpos);
    std::wstring wstrdir = CTypeConvertUtil::StringToWstring(m_strUnInstallDir);

	CDuiString hintString;
	hintString.Format(L"是否删除<%s>及其所有子文件夹?请注意备份保存好工程文件。", wstrdir.c_str());
	int ret = MessageBox(GetHWND(), hintString, L"提示", MB_OKCANCEL);
	if (ret == IDCANCEL)
	{
        //设置进度条;
        m_pUnInstallprogress->SetValue(100);
        CDuiString str;
        str.Format(_T("卸载取消"));
        m_pUnInstallprogress->SetText(str.GetData());
        SetTimer(this->GetHWND(), WMPROGRESSFINISH_TIMER, PORGRESSHIDESEPLEN, 0);
        return;
    }

	int nUnInstall = 0;  //要删除文件个数;
	int ndelindex = 0;	 //已删除文件个数;

	KillProcess();
	//计算卸载数量(文件夹为单位);
	GetUnInstallSize(wstrdir, nUnInstall);
	//删除文件();
	DeleteAllFile(wstrdir, ndelindex, nUnInstall);

	//设置进度条;
	m_pUnInstallprogress->SetValue(100);
	CDuiString str;
	str.Format(_T("已卸载%d%%"), 100);
	m_pUnInstallprogress->SetText(str.GetData());
	SetTimer(this->GetHWND(), WMPROGRESSFINISH_TIMER, PORGRESSHIDESEPLEN, 0);
}

void CMainFrame::GetUnInstallSize(std::wstring &strFile, int &nUnInstall)
{
	if (strFile[strFile.length() - 1] != '\\')
	{
		strFile.append(1, '\\');
	}
	std::wstring szDir = strFile + L"*.*";

	BOOL bRet;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile(szDir.c_str(), &FindFileData);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		bRet = ::FindNextFile(hFind, &FindFileData);
		if (!bRet) break;

		if (FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && !(FindFileData.cFileName[0] == '.'))
		{
			//如果是一个子目录，用递归继续往深一层找 ;
			std::wstring strPath(FindFileData.cFileName);
			strPath = strFile + strPath;
			GetUnInstallSize(strPath, nUnInstall);
		}
	}
	if (strFile[strFile.length() - 1] == '\\')
	{
		strFile = strFile.substr(0, strFile.length() - 1);
	}
	FindClose(hFind);
	nUnInstall++;
}

void CMainFrame::DeleteAllFile(std::wstring &strFile, int &nCurUnInstall, int nUnInstall)
{
	if (strFile[strFile.length() - 1] != '\\')
	{
		strFile.append(1, '\\');
	}
	std::wstring szDir = strFile + L"*.*";

	BOOL bRet;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile(szDir.c_str(), &FindFileData);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		bRet = ::FindNextFile(hFind, &FindFileData);
		if (!bRet) break;

		if (FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && !(FindFileData.cFileName[0] == '.'))
		{
			//如果是一个子目录，用递归继续往深一层找 ;
			std::wstring strPath(FindFileData.cFileName);
			strPath = strFile + strPath;
			DeleteAllFile(strPath, nCurUnInstall, nUnInstall);
		}
		else if (!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && !(FindFileData.cFileName[0] == '.'))
		{
			//显示当前访问的文件(完整路径) ;
			std::wstring strPath(FindFileData.cFileName);
			strPath = strFile + strPath;

			if (_tcsicmp(strPath.c_str(), m_filepath.c_str()) != 0)
			{
				DeleteFile(strPath.c_str());
			}
		}
	}
	if (strFile[strFile.length() - 1] == '\\')
	{
		strFile = strFile.substr(0, strFile.length() - 1);
	}
	FindClose(hFind);
	SetFileAttributes(strFile.c_str(), FILE_ATTRIBUTE_NORMAL);
	RemoveDirectory(strFile.c_str());

	//更新进度条;
	nCurUnInstall++;
	int nprogressvalue = (nCurUnInstall) / (nUnInstall*1.0) * 100 + 15;
	if (nprogressvalue >= 100) nprogressvalue = 98;
	::PostMessage(this->GetHWND(), WM_INSTALLPROGRES_MSG, 0, nprogressvalue);
	Sleep(60);
}

void CMainFrame::KillProcess()
{
	ProcessPrivilege(true);
	//创建进程快照;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe32 = { 0 };
		pe32.dwSize = sizeof(pe32);

		//拿到第一个进程信息;
		if (!Process32First(hSnapshot, &pe32))
		{
			CloseHandle(hSnapshot);
			return;
		}
		do
		{
			CDuiString strKillProcess = APPEXENAME;

			CDuiString strRunProcess = _T("");
			strRunProcess.Format(_T("%s"), pe32.szExeFile);
			if (strKillProcess.GetLength() != strRunProcess.GetLength())
			{
				continue;
			}
			if (_tcsicmp(strKillProcess.GetData(), strRunProcess.GetData()) == 0)
			{
				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
				if (hProcess)
				{
					TerminateProcess(hProcess, 0);//关闭进程;
					CloseHandle(hProcess);
				}
			}

		} while (Process32Next(hSnapshot, &pe32));
	}
	CloseHandle(hSnapshot);
}

BOOL CMainFrame::ProcessPrivilege(BOOL bEnable)
{
	BOOL                   bResult = TRUE;
	HANDLE               hToken = INVALID_HANDLE_VALUE;
	TOKEN_PRIVILEGES     TokenPrivileges;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken) == 0)
	{
		bResult = FALSE;
	}
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &TokenPrivileges.Privileges[0].Luid);
	AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		bResult = FALSE;
	}
	CloseHandle(hToken);

	return bResult;
}