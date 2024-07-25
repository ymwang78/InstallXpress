#include "stdafx.h"
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

static CMainFrame* _sglMainFrame = 0;

CMainFrame::CMainFrame(InstallXpress_Init_t* init_t)
    : m_pInit(init_t)
    , m_bcloseInstall(false)
    , m_pCloseBtn(NULL)
    , m_hThread(NULL)
    , m_pProgress(NULL)
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
	delete this;
}

void CMainFrame::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, DUI_MSGTYPE_WINDOWINIT) == 0) {
		WindowInitialized();
        return;
	}
	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_CLICK) == 0) {
		_OnClickBtn(msg);
    }
	else if (_tcsicmp(msg.sType, DUI_MSGTYPE_SELECTCHANGED) == 0) {
		_OnSelChanged(msg);
	}
    WindowImplBase::Notify(msg);
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
					::KillTimer(this->GetHWND(), WMPROGRESS_TIMER);
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
	if (WM_INSTALLPROGRES_MSG == uMsg) {
        int nNotifyID = (int)wParam;
        if (lParam != (LPARAM) - 1) {
            notify_msg_t* pNotifyMsg = (notify_msg_t*)lParam;
            m_luaPtr->OnUnzipProgress(pNotifyMsg->nNotifyID, pNotifyMsg->totalFileNum, pNotifyMsg->currentFileIndex, pNotifyMsg->totalSize, pNotifyMsg->currentSize);
            delete pNotifyMsg;
            bHandled = TRUE;
        }
        else {
            m_luaPtr->OnUnzipProgress(nNotifyID, -1, -1, -1, -1);
            m_luaPtr->PostSetup();
        }
		return 0L;
	}
	return WindowImplBase::HandleCustomMessage(uMsg, wParam, lParam, bHandled);
}

char* remove_bom(const char* str) {
    if (str == NULL) return NULL;
    const unsigned char* ustr = (const unsigned char*)str;
    if (ustr[0] == 0xEF && ustr[1] == 0xBB && ustr[2] == 0xBF) {
        return strdup(str + 3);
    }
    else {
        return strdup(str);
    }
}

void CMainFrame::WindowInitialized()
{
	SetIcon(m_pInit->nResourceIDIcon);

    m_luaPtr = new InstallLua(&m_PaintManager, UnicodeToUtf8(DirUtility().Version()));
#if 0 //ndef _DEBUG
    m_luaPtr->load_file("Install.lua");
#else
    ResourceHandler* luaScript = LoadResourceFile(m_pInit->nResourceIDLua, _T("LUA_SCRIPT"));
    const char* script = (const char*) luaScript->GetData();
    if (script) {
        m_luaPtr->load_string(remove_bom(script));
    }
    else {
        APPLOG(Log::LOG_ERROR)("Load lua script failed");
    }
#endif
    m_luaPtr->OnInitialize();

	m_pTipLabel = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("errortiplab")));
	m_pCloseBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("closebtn")));
	m_pProgress = static_cast<CProgressUI*>(m_PaintManager.FindControl(_T("installprogress")));

	wchar_t fullHomePath[MAX_PATH] = { 0 };
	m_strCompanyDir = Utf82Unicode(m_luaPtr->QueryByKey_String("InstallPath"));
    _tfullpath(fullHomePath + _tcslen(fullHomePath), m_strCompanyDir.c_str(), sizeof(fullHomePath) / sizeof(fullHomePath[0]));
	m_strCompanyDir = fullHomePath;	

	//if (m_bupdate) InstallSetup(); //更新直接安装;
}

void CMainFrame::_OnClickBtn(TNotifyUI &msg)
{
    m_luaPtr->OnButtonClick(UnicodeToUtf8((LPCTSTR)msg.pSender->GetName()));

	if (_tcsicmp(msg.pSender->GetName(), _T("starinstallbtn")) == 0) {
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

struct InstallXpress_Unzip_Context_t
{
	int nResourceID;
    std::wstring strZipFile;
    std::wstring strUnzipDir;
    int nNotifyID;
	HWND GetHWND() { return _sglMainFrame->GetHWND(); }
	void InstallZip() { 
        if (nResourceID)
            _sglMainFrame->InstallZip(nResourceID, strUnzipDir, nNotifyID);
        else {
            //@todo
            //_sglMainFrame->InstallZip(strZipFile, strUnzipDir, nNotifyID);
        }
    }
};

unsigned int _stdcall CMainFrame::InstallThread(void* param)
{
    InstallXpress_Unzip_Context_t* pThis = (InstallXpress_Unzip_Context_t*)param;
	if (NULL != pThis) {
		SetTimer(pThis->GetHWND(), WMEXPROCESS_TIMER, 1600, NULL);
		pThis->InstallZip();
		::KillTimer(pThis->GetHWND(), WMEXPROCESS_TIMER);
	}
	return 0;
}

int  CMainFrame::UnzipFileAsync(unsigned resourceId, const std::wstring& strUnzipDir, int nNotifyID)
{
	if (0 == FindResource(m_PaintManager.GetResourceDll(), MAKEINTRESOURCE(resourceId), _T("INSTALLSOFT")))
		return -1;
	InstallXpress_Unzip_Context_t* ctx(new InstallXpress_Unzip_Context_t{});
	ctx->nResourceID = resourceId;
	ctx->strUnzipDir = strUnzipDir;
	ctx->nNotifyID = nNotifyID;

	m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CMainFrame::InstallThread, ctx, 0, NULL);
	return 0;
}

int CMainFrame::UnzipFileAsync(const std::wstring& strZipFile, const std::wstring& strUnzipDir, int nNotifyID)
{
    if (!PathFileExists(strZipFile.c_str()))
        return -1;
	InstallXpress_Unzip_Context_t* ctx(new InstallXpress_Unzip_Context_t{});
    ctx->strZipFile = strZipFile;
    ctx->strUnzipDir = strUnzipDir;
    ctx->nNotifyID = nNotifyID;

    m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CMainFrame::InstallThread, ctx, 0, NULL);
	return 0;
}

void CMainFrame::InstallZip(UINT nResourceID, const std::wstring& strUnzipDir, int nNotifyID)
{
	bool bInstallPiu = true;
	bool bNewInstall = false;
	m_pProgress->SetValue(0);

    UINT uId[] = { nResourceID };
    bool bInstallFinish = true;
	////////////////////////////////////	
	for (unsigned i = 0; i < sizeof(uId) / sizeof(uId[0]); ++i) {
        DWORD dwSize = 0;
        ResourceHandler* pInstallContent = LoadResourceFile(uId[i], _T("INSTALLSOFT"));
        if (pInstallContent == NULL)
            return;
        CUnZip7z unzip7z;

		int ret = unzip7z.unzip_7z_file(pInstallContent, m_strCompanyDir, this->GetHWND(), WM_INSTALLPROGRES_MSG, nNotifyID);
        if (ret == 0) 
			continue;
        APPLOG(Log::LOG_ERROR)("\n---Install: 解压失败 ret:%d---\n", ret);
		bInstallFinish = false;
		break;
	}
	if(bInstallFinish) {
        bInstallFinish = true;
        ::PostMessage(this->GetHWND(), WM_INSTALLPROGRES_MSG, nNotifyID, -1);
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
    SetTimer(this->GetHWND(), WMPROGRESSFINISH_TIMER, PORGRESSHIDESEPLEN, 0);
}

void CMainFrame::InstallSetup()
{	
	if (m_pProgress) {
		SetTimer(this->GetHWND(), WMPROGRESS_TIMER, PORGRESSSHOWSEPLEN, 0);
	}
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
