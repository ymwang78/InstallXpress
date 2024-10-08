#pragma once

#include <Utility/DirUtility.h>
#include <Utility/ResourceHandler.h>
class InstallLua;
struct InstallXpress_Init_t;

class CMainFrame final : public WindowImplBase
{
    unsigned _UnzipFileAsyncThread(void* param);
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
	void InstallSetup();
	int  UnzipFileAsync(unsigned resourceId, const std::wstring& strUnzipDir, int nNotifyID);
	int  UnzipFileAsync(const std::wstring& strZipFile, const std::wstring& strUnzipDir, int nNotifyID);
	void InstallZip(UINT nResourceID, const std::wstring& strUnzipDir, int nNotifyID);
	void SplitStringW(const WCHAR *pSrc, WCHAR chMark, std::vector<std::wstring> &vecStrings, BOOL bOnce);
	
public:
	static	unsigned int _stdcall InstallThread(void* param);
	ResourceHandler* LoadResourceFile(UINT uId, LPCTSTR strName);
	const CDirUtility& DirUtility() const { return m_dirUtility; }
	CPaintManagerUI& PaintManager() { return m_PaintManager; }

private: 
    InstallXpress_Init_t* m_pInit;
	bool m_bcloseInstall;
	std::wstring m_strAppExePath;

	CDirUtility  m_dirUtility;
	std::map<UINT, ResourceHandler*> m_resHandlerMap;
	InstallLua* m_luaPtr;

	CButtonUI*	  m_pCloseBtn;
	CLabelUI*     m_pTipLabel;
	CProgressUI*  m_pProgress;

	HANDLE m_hThread;
};

