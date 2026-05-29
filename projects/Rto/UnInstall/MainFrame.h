#pragma once
class InstallLua;
class ResourceHandler;

class CMainFrame : public WindowImplBase
{
public:
	CMainFrame();
	~CMainFrame();

public:
	void InitWindow();
	LPCTSTR GetWindowClassName(void) const;
    ResourceHandler* LoadResourceFile(UINT uId, LPCTSTR lpType);

	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual UILIB_RESOURCETYPE GetResourceType() const;

	virtual void OnFinalMessage(HWND hWnd);
	virtual void Notify(TNotifyUI& msg);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
	static unsigned int _stdcall UnInstallThread(void* param);
	void ClearUnInstall();
	void UnInstall();
	void KillProcess();
	BOOL ProcessPrivilege(BOOL bEnable);
	void GetUnInstallSize(std::wstring &strFile, int &nUnInstall);
	void DeleteAllFile(std::wstring &strFile, int &nCurUnInstall, int nUnInstall);

public:
	HANDLE m_hThread;
	bool m_bUninstall;
	std::wstring m_filepath;
	std::string  m_strUnInstallDir;
	std::wstring m_strname;
	std::wstring m_strplid;
    InstallLua* m_luaPtr;
	CProgressUI*	m_pUnInstallprogress;
	CButtonUI*		m_pStarUnInstallbtn;
	CButtonUI*		m_pUnFinishbtn;
	CButtonUI*      m_pCloseBtn;
    std::map<UINT, ResourceHandler*> m_resHandlerMap;
};

