#pragma once

#include <Utility/DirUtility.h>
#include <Utility/ResourceHandler.h>
class InstallLua;
struct InstallXpress_Init_t;

class CMainFrame final : public WindowImplBase
{
public:
	CMainFrame(InstallXpress_Init_t* init_t);
	~CMainFrame();
	static CMainFrame* GetInstance();

public:
	void InitWindow();
	LPCTSTR GetWindowClassName(void) const;
	UILIB_RESOURCETYPE GetResourceType() const;
	CDuiString GetSkinFile();
	CDuiString GetSkinFolder();
	LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
	CControlUI* CreateControl(LPCTSTR pstrClass);
	void OnFinalMessage(HWND hWnd);
	void Notify(TNotifyUI& msg);

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
	void WindowInitialized();
	void _OnClickBtn(TNotifyUI &msg);
	void _OnSelChanged(TNotifyUI &msg);
	void VisibleCustomRect();
	bool CheckDiskSpace(const std::wstring &strInstallDir);
	bool CheckInstallSize();
	void InstallSetup();
	void InstallZip();
	void UpdateProcess(int nprocess);
	void SplitStringW(const WCHAR *pSrc, WCHAR chMark, std::vector<std::wstring> &vecStrings, BOOL bOnce);
	
public:
	static	unsigned int _stdcall InstallThread(void* param);
	ResourceHandler* LoadResourceFile(UINT uId, LPCTSTR strName);
	const CDirUtility& DirUtility() const { return m_dirUtility; }
	const std::wstring& CompanyDir() const { return m_strCompanyDir; }
	CPaintManagerUI& PaintManager() { return m_PaintManager; }

private: 
    InstallXpress_Init_t* m_pInit;
	//bool m_bupdate;
	bool m_bcustom;
	bool m_bFinish;
	bool m_bcloseInstall;
	int  m_nZipFileNum;
	int  m_nProcess;
	int  m_nOldProcess;
	std::wstring m_strCompanyDir;
	std::wstring m_strAppExePath;

	CDirUtility  m_dirUtility;
	std::map<UINT, ResourceHandler*> m_resHandlerMap;
	InstallLua* m_luaPtr;

	CRichEditUI*  m_pRegText;
	CEditUI*	  m_pInstallEdit;
	CButtonUI*	  m_pCustombtn;
	CButtonUI*    m_pStarinstallbtn;
	CButtonUI*	  m_pCloseBtn;
	CLabelUI*     m_pTipLabel;
	CTabLayoutUI* m_pTabLayout;
	CHorizontalLayoutUI* m_pCustomlayout;
	CVerticalLayoutUI* m_pmainlayout;
	CProgressUI*  m_pProgress;

	HANDLE m_hThread;
};

