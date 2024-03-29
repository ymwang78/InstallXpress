﻿#include "stdafx.h"
#include "MainFrame.h"
#include "Utility\DirUtility.h"
#include "Utility\RegOperation.h"
#include "Utility\TypeConvertUtil.h"
#include "Utility\IniOperation.h"
#include "UnZip7z.h"
#include "Utility/log.h"
#include "LuaExtention.h"
#include "InstallXpress.h"

#include <ShlObj.h>
#include <process.h>
#include <shellapi.h>
#include <TlHelp32.h>

#define  LEFTOFFSET 26            //进度条左侧距离;
#define  BUTTONMINSIZE 200		  //按钮最小值;
#define  PORGRESSSHOWSEPLEN 50	  //进度条显示定时器步长;
#define  PORGRESSHIDESEPLEN 20	  //进度条隐藏定时器步长;

#define  WMPROGRESS_TIMER WM_USER+1000
#define  WMPROGRESSFINISH_TIMER WM_USER+1001
#define  WMEXPROCESS_TIMER WM_USER+1002
#define  WMBKCHANGE_TIMER	WM_USER+1003

#define  MOVESIZE 20
#define  WM_INSTALLPROGRES_MSG WM_USER+2000
#define  WM_INSTALLSITEPROGRES_MSG WM_USER+2001

HANDLE ghEvent = NULL; //全局变量;
static CMainFrame* _sglMainFrame = 0;

CMainFrame::CMainFrame(InstallXpress_Init_t* init_t)
    : m_pInit(init_t)
    , m_pCustomlayout(NULL)
    , m_pCustombtn(NULL)
    , m_bcustom(true)
    , m_bFinish(false)
    , m_bcloseInstall(false)
    , m_pCloseBtn(NULL)
    , m_pInstallEdit(NULL)
    , m_pRegText(NULL)
    , m_pStarinstallbtn(NULL)
    , m_hThread(NULL)
    , m_pProgress(NULL)
    , m_nZipFileNum(0)
    , m_nProcess(35)
    , m_nOldProcess(35)
    , m_dirUtility()
    , m_luaPtr(0)
{
    _sglMainFrame = this;
	m_strCompanyDir = _T("");

}

CMainFrame::~CMainFrame()
{
	PostQuitMessage(0);
}

CMainFrame* CMainFrame::GetInstance()
{
	return _sglMainFrame;
}

void CMainFrame::InitWindow()
{
	CenterWindow();
}

LPCTSTR CMainFrame::GetWindowClassName(void) const
{
	return _T("InstallXpressMainFrame");
}

UILIB_RESOURCETYPE CMainFrame::GetResourceType() const
{
	return UILIB_RESOURCE;
}

CDuiString CMainFrame::GetSkinFile()
{
    return CDuiString(m_pInit->szSkinXML);
}

CDuiString CMainFrame::GetSkinFolder()
{
	return _T("");
}

LRESULT CMainFrame::ResponseDefaultKeyEvent(WPARAM wParam)
{
	return 0;
}

CControlUI* CMainFrame::CreateControl(LPCTSTR pstrClass)
{
	return WindowImplBase::CreateControl(pstrClass);
}

void CMainFrame::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	if (NULL != m_hThread)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	if (ghEvent) {
		CloseHandle(ghEvent);
		ghEvent = NULL;
	}
	delete this;
}

void CMainFrame::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, DUI_MSGTYPE_WINDOWINIT) == 0)
	{
		WindowInitialized();
	}
	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_CLICK) == 0)
	{
		_OnClickBtn(msg);
	}
	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_SELECTCHANGED) == 0)
	{
		_OnSelChanged(msg);
	}
}

LRESULT CMainFrame::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_TIMER)
	{
		if (wParam == WMPROGRESS_TIMER)
		{
			if (NULL == m_pProgress)
			{
				::KillTimer(this->GetHWND(), WMPROGRESS_TIMER);
			}
			else
			{
				RECT rc = m_pProgress->GetPos();
				if (rc.left - MOVESIZE < LEFTOFFSET)
				{
					m_pProgress->SetValue(15);
					m_nProcess = 15;
					m_nOldProcess = 15;
					::KillTimer(this->GetHWND(), WMPROGRESS_TIMER);
					SetEvent(ghEvent);					
					return 0;
				}
				rc.left -= MOVESIZE;
				rc.right += MOVESIZE;
				m_pProgress->SetPos(rc);
			}
		}
		else if (WMEXPROCESS_TIMER == wParam)
		{
			//if (m_nOldProcess == m_nProcess && m_nProcess < 99)
			//{
			//	int nProcess = m_nProcess + 1;
			//	nProcess = nProcess > 100 ? 100 : nProcess;
			//	UpdateProcess(nProcess);
			//}
		}
		else if (WMPROGRESSFINISH_TIMER == wParam)
		{
			if (NULL == m_pProgress)
			{
				::KillTimer(this->GetHWND(), WMPROGRESSFINISH_TIMER);
				return 0;
			}

			RECT rc = m_pProgress->GetPos();
			if (rc.right - rc.left <= BUTTONMINSIZE)
			{
				::KillTimer(this->GetHWND(), WMPROGRESSFINISH_TIMER);

				CHorizontalLayoutUI *pHlayout = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("finishlayout")));
				if (pHlayout) pHlayout->SetVisible(true);

				CButtonUI* pstarusebtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("starusebtn")));
				if (pstarusebtn) pstarusebtn->SetVisible(true);
				if (m_pProgress) m_pProgress->SetVisible(false);
				if (m_pCloseBtn) m_pCloseBtn->SetEnabled(true);
				m_bFinish = true;
				return 0;
			}
			rc.left += MOVESIZE;
			rc.right -= MOVESIZE;
			m_pProgress->SetPos(rc);
		}
	}
	return WindowImplBase::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CMainFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (WM_INSTALLPROGRES_MSG == uMsg)
	{
		int nprocess = (int)(((float)wParam + 1) / (float)(m_nZipFileNum) * 100);
		if (nprocess == 100 && (int)wParam + 1 < m_nZipFileNum)
			nprocess = 99;
		UpdateProcess(nprocess);
	}
	return WindowImplBase::HandleCustomMessage(uMsg, wParam, lParam, bHandled);
}

void CMainFrame::WindowInitialized()
{
	ghEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (ghEvent) {
		//开启线程检测安装大小;
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CMainFrame::InstallThread, this, 0, NULL);
	}
	SetIcon(m_pInit->nResourceIDIcon);

    m_luaPtr = new InstallLua(&m_PaintManager, UnicodeToUtf8(DirUtility().Version()));
#ifdef _DEBUG
    m_luaPtr->load_file("Install.lua");
#else
    ResourceHandler* luaScript = LoadResourceFile(m_pInit->nResourceIDLua, _T("LUA_SCRIPT"));
    m_luaPtr->load_string((const char*)luaScript->GetData());
#endif
    m_luaPtr->OnInitialize();

	m_pCustombtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("custombtn")));
	m_pCustomlayout = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("customlayout")));
	m_pInstallEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("pathedit")));
	m_pRegText = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("portcontent")));
	m_pStarinstallbtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("starinstallbtn")));
	m_pTipLabel = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("errortiplab")));
	m_pCloseBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("closebtn")));
	m_pProgress = static_cast<CProgressUI*>(m_PaintManager.FindControl(_T("installprogress")));

	wchar_t fullHomePath[MAX_PATH] = { 0 };
	m_strCompanyDir = Utf82Unicode(m_luaPtr->QueryByKey_String("InstallPath"));
    _tfullpath(fullHomePath + _tcslen(fullHomePath), m_strCompanyDir.c_str(), sizeof(fullHomePath) / sizeof(fullHomePath[0]));
	m_strCompanyDir = fullHomePath;	
	if (m_pInstallEdit) m_pInstallEdit->SetText(m_strCompanyDir.c_str());

	//if (m_bupdate) InstallSetup(); //更新直接安装;
}

void CMainFrame::_OnClickBtn(TNotifyUI &msg)
{
    m_luaPtr->OnButtonClick(UnicodeToUtf8((LPCTSTR)msg.pSender->GetName()));

	if (_tcsicmp(msg.pSender->GetName(), _T("starinstallbtn")) == 0)
	{
		InstallSetup();
	}
	else if (_tcsicmp(msg.pSender->GetName(), _T("starusebtn")) == 0) {
        PostMessage(WM_CLOSE, 0, 0);
	}
}

void CMainFrame::_OnSelChanged(TNotifyUI &msg)
{
    COptionUI* pCheck = static_cast<COptionUI*>(msg.pSender);
    if (pCheck) {
        m_luaPtr->OnSelChanged(UnicodeToUtf8((LPCTSTR)msg.pSender->GetName()),
            pCheck->IsSelected());
    }
}

void CMainFrame::VisibleCustomRect()
{
    return;
	if (m_bcustom)
	{
		SetWindowPos(this->GetHWND(), NULL, 0, 0, m_PaintManager.GetMaxInfo().cx, m_PaintManager.GetMaxInfo().cy, SWP_NOMOVE | SWP_NOZORDER);
	}
	else
	{
		SetWindowPos(this->GetHWND(), NULL, 0, 0, m_PaintManager.GetMinInfo().cx, m_PaintManager.GetMinInfo().cy, SWP_NOMOVE | SWP_NOZORDER);
	}
	m_bcustom = !m_bcustom;
}

bool CMainFrame::CheckDiskSpace(const std::wstring &strInstallDir)
{
	CLabelUI* pLabelError = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("errortiplab")));
	//检查系统目录;	
	{
		TCHAR str[MAX_PATH] = { 0 };
		GetSystemDirectory(str, MAX_PATH);
		std::wstring strsysdir(str);
		std::wstring strsysdisk = strsysdir.substr(0, 3);
		{
			unsigned _int64 i64FreeBytesToCaller = 0;
			unsigned _int64 i64TotalBytes = 0;
			unsigned _int64 i64FreeBytes = 0;
			bool fResult = GetDiskFreeSpaceEx(strsysdisk.c_str(), (PULARGE_INTEGER)&i64FreeBytesToCaller, (PULARGE_INTEGER)&i64TotalBytes, (PULARGE_INTEGER)&i64FreeBytes);
			unsigned _int64 ufreebytes = i64FreeBytes / 1024 / 1024;
			if (fResult == true && ufreebytes < 300)
			{
				CDuiString strtext = _T("系统盘空间不足,无法完成安装,请清理磁盘空间!");
				if (pLabelError) pLabelError->SetText(strtext);
				return false;
			}
		}
	}

	//检查安装盘大小;
	{
		std::wstring strInstallDisk = strInstallDir.substr(0, 3);
		unsigned _int64 i64FreeBytesToCaller = 0;
		unsigned _int64 i64TotalBytes = 0;
		unsigned _int64 i64FreeBytes = 0;
		bool fResult = GetDiskFreeSpaceEx(strInstallDisk.c_str(), (PULARGE_INTEGER)&i64FreeBytesToCaller, (PULARGE_INTEGER)&i64TotalBytes, (PULARGE_INTEGER)&i64FreeBytes);
		unsigned _int64 ufreebytes = i64FreeBytes / 1024 / 1024;
		if (fResult == true && ufreebytes < 600)
		{
			CDuiString strtext = _T("");
			strtext.Format(_T("%s 盘空间不足,无法完成安装,请更换安装目录或清理磁盘空间!"), strInstallDisk.c_str());
			if (pLabelError) pLabelError->SetText(strtext);
			return false;
		}
	}
	if (pLabelError) pLabelError->SetText(_T(""));
	return true;
}

unsigned int _stdcall CMainFrame::InstallThread(void* param)
{
	CMainFrame* pThis = (CMainFrame*)param;
	if (NULL != pThis)
	{
		pThis->CheckInstallSize();
		WaitForSingleObject(ghEvent, INFINITE);

		if (pThis->m_bcloseInstall) 
			return 0;

		SetTimer(pThis->GetHWND(), WMEXPROCESS_TIMER, 1600, NULL);
		pThis->InstallZip();
		::KillTimer(pThis->GetHWND(), WMEXPROCESS_TIMER);
	}
	return 0;
}

void CMainFrame::InstallZip()
{
	bool bInstallPiu = true;
	bool bNewInstall = false;
	//if (!m_bupdate)
	//	m_pProgress->SetText(_T("正在安装18%..."));
	//else
	//	m_pProgress->SetText(_T("正在更新18%..."));
    m_pProgress->SetText(_T("正在安装18%..."));
	m_pProgress->SetValue(m_nProcess);

    UINT uId[] = {134};// { IDR_INSTALLSOFT1 };
    bool bInstallFinish = true;
	////////////////////////////////////	
	for (unsigned i = 0; i < sizeof(uId) / sizeof(uId[0]); ++i) {
        DWORD dwSize = 0;
        ResourceHandler* pInstallContent = LoadResourceFile(uId[i], _T("INSTALLSOFT"));
        if (pInstallContent == NULL)
            return;
        CUnZip7z unzip7z;

		int ret = unzip7z.unzip_7z_file(pInstallContent, m_strCompanyDir, this->GetHWND(), WM_INSTALLPROGRES_MSG);
        if (ret == 0) 
			continue;
        APPLOG(Log::ERRORLOG)("\n---Install: 解压失败 ret:%d---\n", ret);
		bInstallFinish = false;
		break;
	}
	if(bInstallFinish) {
		UpdateProcess(100);
		bInstallFinish = true;
	}
	else {
		CDuiString str =/* m_bupdate ? _T("更新失败") :*/ _T("安装失败");
		if (bNewInstall) {
			if (m_pTipLabel) m_pTipLabel->SetText(str.GetData());
		}
		else {
			m_pProgress->SetText(str.GetData());
		}
		if (m_pCloseBtn) m_pCloseBtn->SetEnabled(true);
		return;
	}
    m_luaPtr->PostSetup();
    SetTimer(this->GetHWND(), WMPROGRESSFINISH_TIMER, PORGRESSHIDESEPLEN, 0);
}

void CMainFrame::UpdateProcess(int nprocess)
{
	m_nProcess = nprocess > m_nProcess ? nprocess : m_nProcess;
	m_nProcess = m_nProcess > 100 ? 100 : m_nProcess;
	m_pProgress->SetValue(m_nProcess);

	CDuiString str;
	//if (!m_bupdate)
	//	str.Format(_T("正在安装%d%%"), m_nProcess);
	//else
	//	str.Format(_T("正在更新%d%%"), m_nProcess);
    str.Format(_T("正在安装%d%%"), m_nProcess);
	m_pProgress->SetText(str.GetData());
	m_nOldProcess = nprocess;
}

void CMainFrame::InstallSetup()
{	
	if (m_pProgress) {
		SetTimer(this->GetHWND(), WMPROGRESS_TIMER, PORGRESSSHOWSEPLEN, 0);
	}
}

bool CMainFrame::CheckInstallSize()
{
	////////////////////////检测站点包大小/////////////////////;
    UINT uId[] = { 134 };// { IDR_INSTALLSOFT1 };
	for (unsigned i = 0; i < sizeof(uId) / sizeof(uId[0]); ++i) {
        ResourceHandler* pInstallContent = LoadResourceFile(uId[i], _T("INSTALLSOFT"));
        if (pInstallContent == NULL)
            return false;

        CUnZip7z unzip7z;
        int nsize = unzip7z.getunzipfilenum(pInstallContent);

        m_nZipFileNum += (nsize == 0 ? 1500 : nsize);
	}

	return true;
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

void CMainFrame::SplitStringW(const WCHAR *pSrc, WCHAR chMark, std::vector<std::wstring> &vecStrings, BOOL bOnce)
{
	vecStrings.clear();

	if (!pSrc)
		return;

	const WCHAR *pFront = pSrc;
	const WCHAR *pBack = pSrc;
	const WCHAR *pEnd = pSrc + (wcslen(pSrc) - 1);

	if (FALSE == bOnce)
	{
		for (; pBack <= pEnd; ++pBack)
		{
			if (*pBack == chMark)
			{
				vecStrings.push_back(std::wstring(pFront, pBack - pFront));
				pFront = pBack + 1;
			}
			else if (pBack == pEnd)
			{
				vecStrings.push_back(pFront);
				break;
			}
		}
	}
	else
	{
		for (; pBack <= pEnd; ++pBack)
		{
			if (*pBack == chMark)
			{
				vecStrings.push_back(std::wstring(pFront, pBack - pFront));
				pFront = pBack + 1;
				vecStrings.push_back(pFront);
				break;
			}
		}
	}
}
