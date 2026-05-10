#include "stdafx.h"
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "Wininet.lib")
#include "Utility/TypeConvertUtil.h"
#include "LuaExtention.h"
#include <ShlObj_core.h>
#include <sstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include <atlbase.h>

#include "InstallXpress.h"
#include "MainFrame.h"
#include "Utility/log.h"

#define DEBUG_LUAEXT

static DuiLib::CPaintManagerUI* _pPaintManager = NULL;

static std::string _strVersion;

// Path to the per-installer state INI file (set during InstallLua construction)
static std::wstring _strStateFile;

static std::string luas_dumpstack(lua_State* L, const char* fname)
{
    int i, top;
    std::stringstream ss;
    ss << "dumpstack: 0x" << std::hex << L << " " << fname << std::endl;
    top = lua_gettop(L);
    for (i = top; i > 0; --i) {
        ss << "\t[" << i << "][" << luaL_typename(L, i) << "]: " << luaL_tolstring(L, i, NULL) << std::endl;
        lua_pop(L, 1);
    }
    return ss.str();
}

const HKEY _hRootKeyID[] = {
    HKEY_CLASSES_ROOT,
    HKEY_CURRENT_USER,
    HKEY_LOCAL_MACHINE,
    HKEY_USERS,
    HKEY_CURRENT_CONFIG
};

// 跨平台mkdir函数
inline int portable_mkdir(const TCHAR* path) {
#ifdef _WIN32
    return _wmkdir(path);
#else 
    return mkdir(path, 0755); // 使用Unix/Linux权限模式
#endif
}

bool mkdir_p(const std::wstring& path) {
    size_t pos = 0;
    std::wstring currentPath;
    std::wstring delimiter = L"/";

#ifdef _WIN32
    delimiter = L"\\";
#endif

    // 去除路径末尾的分隔符
    std::wstring normalizedPath = path;
    if (normalizedPath.back() == delimiter.back()) {
        normalizedPath.pop_back();
    }

    while ((pos = normalizedPath.find(delimiter, pos)) != std::string::npos) {
        currentPath = normalizedPath.substr(0, pos++);
        if (currentPath.empty()) continue; // 如果是绝对路径，第一个会是空的

        if (portable_mkdir(currentPath.c_str()) && errno != EEXIST) {
            return false;
        }
    }

    if (portable_mkdir(normalizedPath.c_str()) && errno != EEXIST) {
        return false;
    }

    return true;
}

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
    if (uMsg == BFFM_INITIALIZED) SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
    return 0;
}

CDuiString BrowseForFolder(HWND hwnd, CDuiString title, CDuiString folder)
{
    CDuiString ret;

    BROWSEINFO br;
    ZeroMemory(&br, sizeof(BROWSEINFO));
    br.lpfn = BrowseCallbackProc;
    br.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    br.hwndOwner = hwnd;
    br.lpszTitle = title;
    br.lParam = (LPARAM)(LPCTSTR)folder;

    LPITEMIDLIST pidl = NULL;
    if ((pidl = SHBrowseForFolder(&br)) != NULL) {
        wchar_t buffer[MAX_PATH];
        if (SHGetPathFromIDList(pidl, buffer)) ret = buffer;
    }

    return ret;
}



extern "C"
static int l_DiskFreeSpace(lua_State * L)
{
    const char* path = luaL_checkstring(L, 1);
    if (path == nullptr) {
        lua_pushnil(L);
        return 1;
    }
    ULARGE_INTEGER freeBytesAvailable;
    ULARGE_INTEGER totalNumberOfBytes;
    ULARGE_INTEGER totalNumberOfFreeBytes;

    if (path[0] == 0) {
        TCHAR str[MAX_PATH] = { 0 };
        GetSystemDirectory(str, MAX_PATH);
        if (GetDiskFreeSpaceEx(str, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
            lua_pushinteger(L, freeBytesAvailable.QuadPart);
            return 1;
        }
    }
    else {
        if (GetDiskFreeSpaceEx(Utf82Unicode(path).c_str(), &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
            lua_pushinteger(L, freeBytesAvailable.QuadPart);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;

}

extern "C"
static int l_DuiEnable(lua_State * L)
{
    const char* ctrlName = luaL_checkstring(L, 1);
    DuiLib::CControlUI* pControl = dynamic_cast<DuiLib::CControlUI*>(
        _pPaintManager == 0 ? 0 : _pPaintManager->FindControl(Utf82Unicode(ctrlName).c_str()));
    if (ctrlName == nullptr || pControl == 0 || _pPaintManager == 0) {
        lua_pushboolean(L, false);
        return 1;
    }
    if (lua_isboolean(L, 2)) {
        pControl->SetEnabled(lua_toboolean(L, 2));
        lua_pushboolean(L, true);
        return 1;
    }
    else {
        lua_pushboolean(L, pControl->IsEnabled());
        return 1;
    }
}

extern "C"
static int l_DuiMessage(lua_State* L)
{
    lua_Integer message_id = lua_tointeger(L, 1);
    lua_Integer message_wparam = lua_tointeger(L, 2);
    lua_Integer message_lparam = lua_tointeger(L, 3);
    if (_pPaintManager == 0) {
        lua_pushboolean(L, false);
        return 1;
    }
    ::PostMessage(_pPaintManager->GetPaintWindow(), (UINT)message_id, (WPARAM)message_wparam, (LPARAM)message_lparam);
    return 0;
}

extern "C"
static int l_DuiOptionSelect(lua_State* L)
{
    const char* btnName = luaL_checkstring(L, 1);
    DuiLib::COptionUI* pOptionUI = dynamic_cast<DuiLib::COptionUI*>(
        _pPaintManager == 0 ? 0 : _pPaintManager->FindControl(Utf82Unicode(btnName).c_str()));
    if (btnName == nullptr || pOptionUI == 0 || _pPaintManager == 0) {
        lua_pushboolean(L, false);
        return 1;
    }
    if (lua_isboolean(L, 2)) {
        pOptionUI->Selected(lua_toboolean(L, 2));
        lua_pushboolean(L, true);
        return 1;
    }
    else {
        lua_pushboolean(L, pOptionUI->IsSelected());
        return 1;
    }
}

extern "C"
static int l_DuiProgress(lua_State * L)
{
    const char* ctrlName = luaL_checkstring(L, 1);
    if (ctrlName == nullptr || _pPaintManager == 0) {
        lua_pushnil(L);
        return 1;
    }
    DuiLib::CProgressUI* pControl = dynamic_cast<DuiLib::CProgressUI*>(_pPaintManager->FindControl(Utf82Unicode(ctrlName).c_str()));
    if (pControl == 0) {
        if (lua_isnone(L, 2))
            lua_pushinteger(L, 0);
        else
            lua_pushboolean(L, false);
        return 1;
    }
    if (lua_isnone(L, 2)) {
        lua_pushinteger(L, pControl->GetValue());
        return 1;
    }
    else if (lua_isinteger(L, 2)) {
        pControl->SetValue((int)lua_tointeger(L, 2));
        lua_pushboolean(L, true);
        if (lua_isstring(L, 3)) {
            const char* text = luaL_checkstring(L, 3);
            pControl->SetText(Utf82Unicode(text).c_str());
        }
        return 1;
    }
    lua_pushboolean(L, false);
    return 1;
}

extern "C"
static int l_DuiSetBkImage(lua_State * L)
{
    const char* ctrlName = luaL_checkstring(L, 1);
    const char* imgDesc = luaL_checkstring(L, 2);
    if (ctrlName == nullptr || imgDesc == nullptr || _pPaintManager == 0) {
        lua_pushboolean(L, false);
        return 1;
    }
    DuiLib::CControlUI* pControl = dynamic_cast<DuiLib::CControlUI*>(_pPaintManager->FindControl(Utf82Unicode(ctrlName).c_str()));
    if (pControl) {
        pControl->SetBkImage(Utf82Unicode(imgDesc).c_str());
        lua_pushboolean(L, true);
        return 1;
    }
    lua_pushboolean(L, false);
    return 1;
}

extern "C"
static int l_DuiText(lua_State * L)
{
    const char* ctrlName = luaL_checkstring(L, 1);
    if (ctrlName == nullptr || _pPaintManager == 0) {
        lua_pushboolean(L, false);
        return 1;
    }
    DuiLib::CControlUI* pControl = dynamic_cast<DuiLib::CControlUI*>(_pPaintManager->FindControl(Utf82Unicode(ctrlName).c_str()));
    if (pControl == 0) {
        if (lua_isnil(L, 2))
            lua_pushstring(L, "");
        else
            lua_pushboolean(L, false);
        return 1;
    }
    if (lua_isnil(L, 2)) {
        CDuiString text = pControl->GetText();
        lua_pushstring(L, UnicodeToUtf8(std::wstring(text)).c_str());
        return 1;
    }
    else if (lua_isinteger(L, 2)) {
        do {
            int nID = (int)lua_tointeger(L, 2);
            const wchar_t* rcType = L"REGCONTENT";
            std::wstring rcStr;
            if (lua_isstring(L, 3)) {
                rcStr = Utf82Unicode(lua_tostring(L, 3));
                rcType = rcStr.c_str();
            }
            HRSRC hResource = ::FindResource(_pPaintManager->GetResourceDll(), MAKEINTRESOURCE(nID), rcType);
            if (hResource == NULL)
                break;
            DWORD dwSize = 0;
            HGLOBAL hGlobal = ::LoadResource(_pPaintManager->GetResourceDll(), hResource);
            if (hGlobal == NULL) {
#if defined(WIN32) && !defined(UNDER_CE)
                ::FreeResource(hResource);
#endif
                break;
            }
            dwSize = ::SizeofResource(_pPaintManager->GetResourceDll(), hResource);
            if (dwSize == 0) break;

            std::unique_ptr<char> pRegContent(new char[dwSize + 1]);
            memset(pRegContent.get(), 0, dwSize + 1);

            if (pRegContent.get() != NULL) {
                ::CopyMemory(pRegContent.get(), (char*)::LockResource(hGlobal), dwSize);
            }
#if defined(WIN32) && !defined(UNDER_CE)
            ::FreeResource(hResource);
#endif
            if (pRegContent.get() == NULL) break;
            std::string strText(pRegContent.get());

            std::wstring wstrContent = CTypeConvertUtil::StringToWstring(strText);
            pControl->SetText(wstrContent.c_str());

            lua_pushboolean(L, true);
            return 1;

        } while (0);

        lua_pushboolean(L, 0);
        return 1;
    }
    else if (lua_isstring(L, 2)) {
        const char* text = luaL_checkstring(L, 2);
        pControl->SetText(Utf82Unicode(text).c_str());
        lua_pushboolean(L, true);
        return 1;
    }
    lua_pushboolean(L, false);
    return 1;
}

extern "C"
static int l_DuiTextColor(lua_State * L)
{
    const char* ctrlName = luaL_checkstring(L, 1);
    if (ctrlName == nullptr || _pPaintManager == 0) {
        lua_pushboolean(L, false);
        return 1;
    }
    DuiLib::CControlUI* pControl = dynamic_cast<DuiLib::CControlUI*>(_pPaintManager->FindControl(Utf82Unicode(ctrlName).c_str()));
    if (pControl == 0) {
        if (lua_isnone(L, 2))
            lua_pushinteger(L, 0);
        else
            lua_pushboolean(L, false);
        return 1;
    }
    if (lua_isnone(L, 2)) {
        lua_pushinteger(L, pControl->GetTextColor());
        return 1;
    }
    else if (lua_isinteger(L, 2)) {
        pControl->SetTextColor((DWORD)lua_tointeger(L, 2));
        lua_pushboolean(L, true);
        return 1;
    }
    lua_pushboolean(L, false);
    return 1;

}

extern "C"
static int l_DuiVisible(lua_State * L)
{
    const char* ctrlName = luaL_checkstring(L, 1);
    if (ctrlName == nullptr || _pPaintManager == 0) {
        lua_pushboolean(L, false);
        return 1;
    }
    DuiLib::CControlUI* pControl = dynamic_cast<DuiLib::CControlUI*>(_pPaintManager->FindControl(Utf82Unicode(ctrlName).c_str()));

    if (pControl) {
        if (lua_isboolean(L, 2)) {
            pControl->SetVisible(lua_toboolean(L, 2));
            lua_pushboolean(L, true);
            return 1;
        }
        else {
            lua_pushboolean(L, pControl->IsVisible());
            return 1;
        }
    }
    lua_pushboolean(L, false);
    return 1;
}

extern "C"
static int l_DuiTabSelect(lua_State * L)
{
    const char* ctrlName = luaL_checkstring(L, 1);
    if (ctrlName == nullptr || _pPaintManager == 0) {
        lua_pushboolean(L, false);
        return 1;
    }

    DuiLib::CTabLayoutUI* pControl = dynamic_cast<DuiLib::CTabLayoutUI*>(_pPaintManager->FindControl(Utf82Unicode(ctrlName).c_str()));
    if (pControl == 0) {
        lua_pushboolean(L, false);
        return 1;
    }

    if (lua_isinteger(L, 2)) {
        lua_pushboolean(L, pControl->SelectItem((int)lua_tointeger(L, 2)));
        return 1;
    }
    else {
        lua_pushinteger(L, pControl->GetCurSel());
        return 1;
    }
}

extern "C"
static int l_DuiWindowPos(lua_State * L)
{
    const char* ctrlName = luaL_checkstring(L, 1);
    if (ctrlName == nullptr || _pPaintManager == 0) {
        lua_pushboolean(L, false);
        return 1;
    }
    if (0 == strcmp(ctrlName, "_MainFrame")) {
        HWND hWnd = _pPaintManager->GetPaintWindow();
        if (!::IsWindow(hWnd)) {
            if (lua_isnil(L, 2))
                lua_pushinteger(L, 0);
            else
                lua_pushboolean(L, false);
            return 1;
        }
        if (lua_isnone(L, 2)) {
            RECT rc{};
            GetWindowRect(hWnd, &rc);
            lua_pushinteger(L, rc.left);
            lua_pushinteger(L, rc.top);
            lua_pushinteger(L, rc.right - rc.left);
            lua_pushinteger(L, rc.bottom - rc.top);
            return 4;
        }
        else if (lua_isinteger(L, 2) && lua_isinteger(L, 3) && lua_isinteger(L, 4) && lua_isinteger(L, 5)) {
            ::SetWindowPos(hWnd, NULL, (LONG)lua_tointeger(L, 2), (LONG)lua_tointeger(L, 3), (LONG)lua_tointeger(L, 4), (LONG)lua_tointeger(L, 5), SWP_NOMOVE | SWP_NOZORDER);
            lua_pushboolean(L, true);
            return 1;
        }
        lua_pushboolean(L, false);
        return 1;
    }
    else { //not tested
        DuiLib::CControlUI* pControl = dynamic_cast<DuiLib::CControlUI*>(_pPaintManager->FindControl(Utf82Unicode(ctrlName).c_str()));
        if (pControl == 0) {
            if (lua_isnone(L, 2))
                lua_pushinteger(L, 0);
            else
                lua_pushboolean(L, false);
            return 1;
        }
        if (lua_isnone(L, 2)) {
            const RECT& rc = pControl->GetPos();
            lua_pushinteger(L, rc.left);
            lua_pushinteger(L, rc.top);
            lua_pushinteger(L, rc.right - rc.left);
            lua_pushinteger(L, rc.bottom - rc.top);
            return 4;
        }
        else if (lua_isinteger(L, 2) && lua_isinteger(L, 3) && lua_isinteger(L, 4) && lua_isinteger(L, 5)) {
            pControl->SetPos(RECT{ (LONG)lua_tointeger(L, 2), (LONG)lua_tointeger(L, 3), (LONG)lua_tointeger(L, 4), (LONG)lua_tointeger(L, 5) });
            lua_pushboolean(L, true);
            return 1;
        }
        lua_pushboolean(L, false);
        return 1;
    }
}


extern "C"
static int l_FilePathChoose(lua_State * L)
{
    const char* title = luaL_checkstring(L, 1);
    const char* defaultPath = luaL_checkstring(L, 2);
    if (title == nullptr || _pPaintManager == 0) {
        lua_pushnil(L);
        return 1;
    }
    if (defaultPath == nullptr) {
        defaultPath = "";
    }
    CDuiString str = BrowseForFolder(_pPaintManager->GetPaintWindow(), Utf82Unicode(title).c_str(), Utf82Unicode(defaultPath).c_str());
    lua_pushstring(L, UnicodeToUtf8(std::wstring(str)).c_str());
    return 1;
}

extern "C"
static int l_FilePathCreateShortCut(lua_State * L)
{
    const char* lnk = luaL_checkstring(L, 1);
    const char* target = luaL_checkstring(L, 2);
    const char* workdir = luaL_checkstring(L, 3);
    const char* description = luaL_checkstring(L, 4);

    APPLOG(LOG_TRACE)("%s: %s\n", "FilePathCreateShortCut", lnk);

    if (lnk == nullptr || target == nullptr || workdir == nullptr || description == nullptr) {
        lua_pushboolean(L, false);
        return 1;
    }

    HRESULT result;
    IShellLink* plink;
    IPersistFile* ppf;
    result = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&plink);
    if (FAILED(result) || NULL == plink) {
        lua_pushboolean(L, false);
        return 1;
    }

    result = plink->QueryInterface(IID_IPersistFile, (void**)&ppf);
    if (FAILED(result)) {
        plink->Release();
        lua_pushboolean(L, false);
        return 1;
    }

    plink->SetPath(Utf82Unicode(target).c_str());
    plink->SetWorkingDirectory(Utf82Unicode(workdir).c_str());
    plink->SetDescription(Utf82Unicode(description).c_str());

    if (SUCCEEDED(ppf->Save(Utf82Unicode(lnk).c_str(), TRUE))) {
        ppf->Release();
        plink->Release();
        lua_pushboolean(L, true);
        return 1;
    }
    ppf->Release();
    plink->Release();
    lua_pushboolean(L, false);
    ::CoUninitialize();
    return 1;
}

extern "C" static int l_FilePathCopy(lua_State* L) {
    const char* srcPath = luaL_checkstring(L, 1);
    const char* destPath = luaL_checkstring(L, 2);
    BOOL overwrite = TRUE;
    if (lua_isboolean(L, 3)) {
        overwrite = lua_toboolean(L, 3) ? TRUE : FALSE;
    }
    APPLOG(LOG_TRACE)("%s: %s -> %s\n", "FilePathCopy", srcPath, destPath);
    if (srcPath == nullptr || destPath == nullptr) {
        lua_pushboolean(L, false);
        return 1;
    }
    BOOL ret = CopyFile(Utf82Unicode(srcPath).c_str(), Utf82Unicode(destPath).c_str(), overwrite);
    lua_pushboolean(L, ret);
    return 1;
}

extern "C"
static int l_FilePathDelete(lua_State * L)
{
    const char* path = luaL_checkstring(L, 1);

    APPLOG(LOG_TRACE)("%s: %s\n", "FilePathDelete", path);

    if (path == nullptr) {
        lua_pushboolean(L, false);
        return 1;
    }
    DWORD attributes = GetFileAttributes(Utf82Unicode(path).c_str());

    if (attributes == INVALID_FILE_ATTRIBUTES) {
        lua_pushboolean(L, false);
        return 1;
    }

    if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
        if (DeleteDirectory(Utf82Unicode(path).c_str())) {
            lua_pushboolean(L, true);
            return 1;
        }
    }
    else {
        if (DeleteFile(Utf82Unicode(path).c_str())) {
            lua_pushboolean(L, true);
            return 1;
        }
    }

    lua_pushboolean(L, false);
    return 1;
}

extern "C"
static int l_FilePathExists(lua_State * L)
{
    const char* path = luaL_checkstring(L, 1);
    if (path == nullptr) {
        lua_pushboolean(L, false);
        return 1;
    }
    DWORD attributes = GetFileAttributes(Utf82Unicode(path).c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        lua_pushboolean(L, false);
        return 1;
    }
    lua_pushboolean(L, true);
    return 1;
}

extern "C"
static int l_FilePathGetSpecialLocation(lua_State * L)
{
    int nFolder = (int)lua_tointeger(L, 1);
    if (nFolder < 0 || nFolder > 0x8000) {
        lua_pushnil(L);
        return 1;
    }

    wchar_t szPath[MAX_PATH] = { 0 };

    if (SUCCEEDED(SHGetFolderPath(NULL,
        nFolder | CSIDL_FLAG_CREATE,
        NULL,
        0,
        szPath)))
    {
        lua_pushstring(L, UnicodeToUtf8(szPath).c_str());
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

extern "C"
static int l_FilePathMakeDir(lua_State * L)
{
    const char* path = luaL_checkstring(L, 1);
    if (path == nullptr) {
        lua_pushboolean(L, false);
        return 1;
    }
    if (mkdir_p(Utf82Unicode(path).c_str())) {
        lua_pushboolean(L, true);
        return 1;
    }
    lua_pushboolean(L, false);
    return 1;

}

extern "C"
static int l_FilePathUnzipAsync(lua_State * L)
{
    if (lua_istable(L, 1)) {
        const char* destDir = luaL_checkstring(L, 2);
        if (destDir == nullptr) {
            lua_pushboolean(L, false);
            return 1;
        }
        std::vector<UINT> resourceIDs;
        std::vector<std::wstring> skipPrefixes;
        lua_pushnil(L);
        while (lua_next(L, 1) != 0) {
            if (lua_isinteger(L, -1))
                resourceIDs.push_back((UINT)lua_tointeger(L, -1));
            lua_pop(L, 1);
        }
        if (lua_istable(L, 3)) {
            lua_pushnil(L);
            while (lua_next(L, 3) != 0) {
                if (lua_isstring(L, -1))
                    skipPrefixes.push_back(Utf82Unicode(lua_tostring(L, -1)));
                lua_pop(L, 1);
            }
        }
        int ret = CMainFrame::GetInstance()->UnzipFileAsync(resourceIDs, Utf82Unicode(destDir), skipPrefixes);
        lua_pushboolean(L, ret >= 0);
        return 1;
    }
    else if (lua_isinteger(L, 1)) {
        unsigned resourceId = (unsigned) lua_tointeger(L, 1);
		const char* destDir = luaL_checkstring(L, 2);
		int notifyID = 0;
		if (resourceId == 0 || destDir == nullptr) {
			lua_pushboolean(L, false);
			return 1;
		}
		if (lua_isinteger(L, 3))
			notifyID = (int)lua_tointeger(L, 3);

		int ret = CMainFrame::GetInstance()->UnzipFileAsync(resourceId, Utf82Unicode(destDir), notifyID);

		lua_pushboolean(L, ret >= 0);
		return 1;
    }
    else if (lua_isstring(L, 1)) {
		const char* zipFile = luaL_checkstring(L, 1);
		const char* destDir = luaL_checkstring(L, 2);
		int notifyID = 0;
		if (zipFile == nullptr || destDir == nullptr) {
			lua_pushboolean(L, false);
			return 1;
		}
		if (lua_isinteger(L, 3))
			notifyID = (int)lua_tointeger(L, 3);

		int ret = CMainFrame::GetInstance()->UnzipFileAsync(Utf82Unicode(zipFile), Utf82Unicode(destDir), notifyID);

		lua_pushboolean(L, ret >= 0);
		return 1;
    }
    lua_pushboolean(L, false);
    return 1;
}

extern "C"
static int l_LogPrint(lua_State * L) 
{
    std::string log;
    int top = lua_gettop(L);
    for (int i = 1; i <= top; ++i) {
        const char* v = lua_tostring(L, i);
        if (v == nullptr) {
            log += "nil";
        }
        else {
            log += v;
        }
    }
    log += "\n";

    APPLOG(LOG_TRACE)("%s", log.c_str());
    return 0;
}

extern "C"
static int l_ProcessExecute(lua_State * L)
{
    bool hidden = true;
    int wait_sec = -1;
    const char* cmd = luaL_checkstring(L, 1);
    std::wstring strCommand = Utf82Unicode(cmd);

    if (lua_isboolean(L, 2))
        hidden = lua_toboolean(L, 2);
    if (lua_isinteger(L, 3))
        wait_sec = (int)lua_tointeger(L, 3);

    APPLOG(LOG_TRACE)("%s: %s\n", "ProcessExecute", strCommand);

    if (cmd == nullptr) {
        lua_pushboolean(L, false);
        return 1;
    }

    DWORD ret = ExecuteProcess(strCommand.c_str(), hidden, wait_sec);
    lua_pushboolean(L, ret != S_FALSE);
    return 1;
}

extern "C"
static int l_ProcessKill(lua_State * L)
{
    const char* processName = luaL_checkstring(L, 1);
    std::wstring strProcessName = Utf82Unicode(processName);

    APPLOG(LOG_TRACE)("%s: %s\n", "ProcessKill", processName);

    BOOL ret = KillProcess(strProcessName.c_str());
    lua_pushboolean(L, ret);
    return 1;
}

extern "C"
static int l_RunAsAdmin(lua_State * L)
{
    BOOL ret = TryElevate();
    lua_pushboolean(L, ret);
    return 1;
}


extern "C"
static int l_RegGetValue(lua_State* L) 
{
    lua_Integer rootKeyID = lua_tointeger(L, 1);
    const char* path = luaL_checkstring(L, 2);
    const char* key = luaL_checkstring(L, 3);

    if (rootKeyID < 0 || rootKeyID >= sizeof(_hRootKeyID) / sizeof(_hRootKeyID[0])) {
        lua_pushnil(L);
        return 1;
    }

    if (path == nullptr || key == nullptr) {
        lua_pushnil(L);
        return 1;
    }

    HKEY hKeyRoot = _hRootKeyID[rootKeyID];

    HKEY hKey;
    if (RegOpenKeyEx(hKeyRoot, Utf82Unicode(path).c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        lua_pushnil(L);
        return 1;
    }

    if (key[0] == 0) { //value为""的情况下只是查询这个项目是否存在
        RegCloseKey(hKey);
        lua_pushboolean(L, true);
        return 1;
    }

    DWORD lValueType = 0;
    unsigned char data[1024];
    DWORD dataSize = sizeof(data);
    if (RegQueryValueEx(hKey, Utf82Unicode(key).c_str(), NULL, &lValueType, (LPBYTE)&data, &dataSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        lua_pushnil(L);
        return 1;
    }

    switch (lValueType) {
    case REG_DWORD: {
        lua_pushinteger(L, *(DWORD*)data);
        break;
    }
    case REG_SZ: {
        lua_pushstring(L, UnicodeToUtf8((wchar_t*)data).c_str());
        break;
    }
    default: {
        lua_pushnil(L);
        break;
    }
    }

    // 返回值
    RegCloseKey(hKey);
    return 1;
}

extern "C"
static int l_RegSetValue(lua_State* L) 
{
    // 获取参数
    lua_Integer rootKeyID = lua_tointeger(L, 1);
    const char* path = luaL_checkstring(L, 2);
    const char* key = luaL_checkstring(L, 3);

    APPLOG(LOG_TRACE)("%s: %s, %s\n", "RegSetValue", path, key);

    if (rootKeyID < 0 || rootKeyID >= sizeof(_hRootKeyID) / sizeof(_hRootKeyID[0])) {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (path == nullptr || key == nullptr) {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (!lua_isinteger(L, 4) && !lua_isstring(L, 4)) {
        lua_pushboolean(L, 0);
        return 1;
    }

    HKEY hKeyRoot = _hRootKeyID[rootKeyID];

    HKEY hKey;

    if (RegCreateKeyEx(hKeyRoot, Utf82Unicode(path).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (lua_isinteger(L, 4)) {
        DWORD value = (DWORD)lua_tointeger(L, 4);
        if (RegSetValueEx(hKey, Utf82Unicode(key).c_str(), 0, REG_SZ, (const BYTE*)value, sizeof(DWORD)) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            lua_pushboolean(L, 0);
            return 1;
        }
    }
    else if (lua_isstring(L, 4)) {
        std::wstring value = Utf82Unicode( lua_tostring(L, 4));
        if (RegSetValueEx(hKey, Utf82Unicode(key).c_str(), 0, REG_SZ, (const BYTE*)value.c_str(), value.size() * 2) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            lua_pushboolean(L, 0);
            return 1;
        }
    }

    RegCloseKey(hKey);
    lua_pushboolean(L, 1);
    return 1;
}

extern "C"
static int l_RegDeleteValue(lua_State * L) 
{
    lua_Integer rootKeyID = lua_tointeger(L, 1);
    const char* path = luaL_checkstring(L, 2);
    const char* key = luaL_checkstring(L, 3);

    APPLOG(LOG_TRACE)("%s: %s, %s\n", "RegDeleteValue", path, key);

    if (rootKeyID < 0 || rootKeyID >= sizeof(_hRootKeyID) / sizeof(_hRootKeyID[0])) {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (path == nullptr || key == nullptr) {
        lua_pushboolean(L, 0);
        return 1;
    }

    HKEY hKeyRoot = _hRootKeyID[rootKeyID];

    HKEY hKey;

    if (RegOpenKeyEx(hKeyRoot, Utf82Unicode(path).c_str(), 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (RegDeleteValue(hKey, Utf82Unicode(key).c_str()) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        lua_pushboolean(L, 0);
        return 1;
    }

    RegCloseKey(hKey);
    lua_pushboolean(L, 1);
    return 1;
}

extern "C"
static int l_RegDeleteKey(lua_State * L)
{
    lua_Integer rootKeyID = lua_tointeger(L, 1);
    const char* path = luaL_checkstring(L, 2);
    const char* key = 0;
    if (lua_isstring(L, 3))
        key = luaL_checkstring(L, 3);

    APPLOG(LOG_TRACE)("%s: %s, %s\n", "RegDeleteKey", path, key ? key : "");

    if (rootKeyID < 0 || rootKeyID >= sizeof(_hRootKeyID) / sizeof(_hRootKeyID[0])) {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (path == nullptr || key == nullptr) {
        lua_pushboolean(L, 0);
        return 1;
    }

    HKEY hKeyRoot = _hRootKeyID[rootKeyID];

    HKEY hKey;

    if (RegOpenKeyEx(hKeyRoot, Utf82Unicode(path).c_str(), 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (RegDeleteTree(hKey, key ? Utf82Unicode(key).c_str() : NULL) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        lua_pushboolean(L, 0);
        return 1;
    }

    RegCloseKey(hKey);
    lua_pushboolean(L, 1);
    return 1;
}

extern "C"
static int l_SysACP(lua_State * L)
{
    lua_pushinteger(L, GetACP());
    return 1;
}


static bool SetSysEnvironmentVariable(const wchar_t* name, const wchar_t* value) {
    HKEY hKey;
    DWORD dwDisposition;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"),
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS) {
        return false;
    }

    if (RegSetValueEx(hKey, name, 0, REG_SZ, (const BYTE*)value, sizeof(wchar_t) * (wcslen(value) + 1)) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);

    SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
    return true;
}

static int l_SysEnvSet(lua_State* L)
{
    if (lua_gettop(L) != 2) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Incorrect number of arguments. Expected 2 (name, value)");
        return 2;
    }

    const char* name = luaL_checkstring(L, 1);
    const char* value = luaL_checkstring(L, 2);
    APPLOG(LOG_TRACE)("%s: %s, %s\n", "SysEnvSet", name, value);

    if (SetSysEnvironmentVariable(Utf82Unicode(name).c_str(), Utf82Unicode(value).c_str())) {
        lua_pushboolean(L, 1); // Return true on success
        return 1;
    }
    else {
        lua_pushboolean(L, 0); // Return false on failure
        lua_pushstring(L, "Failed to set environment variable");
        return 2;
    }
}

static bool isPathPresent(const std::wstring& currentPath, const std::wstring& newPath)
{
    size_t start_pos = 0, commet_pos = 0;
    while ((commet_pos = currentPath.find(';', start_pos)) != std::wstring::npos) {
        if (currentPath.substr(start_pos, newPath.length()) == newPath) {
            return true;
        }
        start_pos = commet_pos + 1;
    }
    return false;
}

static int l_SysPathAdd(lua_State* L)
{
    const char* newPath = luaL_checkstring(L, 1);
    std::wstring new_path_str = Utf82Unicode(newPath);

    CRegKey regKey;
    APPLOG(LOG_TRACE)("%s: %s\n", "SysPathAdd", newPath);

    LONG result = regKey.Open(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"), KEY_READ | KEY_WRITE);
    if (result != ERROR_SUCCESS) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Failed to open registry key");
        return 2;
    }

    DWORD size = 0;
    result = regKey.QueryStringValue(TEXT("Path"), NULL, &size);
    if (result != ERROR_SUCCESS) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Failed to read PATH value");
        return 2;
    }

    std::vector<wchar_t> vec(size + 1, 0);


    result = regKey.QueryStringValue(TEXT("Path"), vec.data(), &size);
    if (result != ERROR_SUCCESS) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Failed to read PATH value");
        return 2;
    }

    std::wstring current_path(vec.data());

    if (isPathPresent(current_path, new_path_str)) {
        lua_pushboolean(L, 1);
        return 1;
    }
    current_path += L";";
    current_path += new_path_str;

    result = regKey.SetStringValue(TEXT("Path"), current_path.c_str(), REG_EXPAND_SZ);
    if (result != ERROR_SUCCESS) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Failed to set new PATH value");
        return 2;
    }

    lua_pushboolean(L, 1);
    return 1;
}

// ── System information queries ────────────────────────────────────────────

extern "C"
static int l_SysIsWin10OrLater(lua_State* L)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi) };
    osvi.dwMajorVersion = 10;
    DWORDLONG mask = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    lua_pushboolean(L, VerifyVersionInfoW(&osvi, VER_MAJORVERSION, mask) != 0);
    return 1;
}

extern "C"
static int l_SysGetArch(lua_State* L)
{
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    switch (si.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64: lua_pushstring(L, "x64");   break;
    case PROCESSOR_ARCHITECTURE_ARM64: lua_pushstring(L, "arm64"); break;
    case PROCESSOR_ARCHITECTURE_INTEL: lua_pushstring(L, "x86");   break;
    default:                           lua_pushstring(L, "unknown"); break;
    }
    return 1;
}

extern "C"
static int l_SysNetworkAvailable(lua_State* L)
{
    DWORD flags = 0;
    lua_pushboolean(L, InternetGetConnectedState(&flags, 0) == TRUE);
    return 1;
}

extern "C"
static int l_SysGetInstalledSoftware(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    if (name == nullptr) { lua_pushnil(L); return 1; }
    std::wstring wname = Utf82Unicode(name);
    const wchar_t* kUninstallSubkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";

    // Query both 64-bit and 32-bit registry views explicitly so the result is
    // correct regardless of whether this process is 32-bit or 64-bit.
    const REGSAM views[] = { KEY_READ | KEY_WOW64_64KEY, KEY_READ | KEY_WOW64_32KEY };
    for (REGSAM sam : views) {
        std::wstring path = std::wstring(kUninstallSubkey) + wname;
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, path.c_str(), 0, sam, &hKey) == ERROR_SUCCESS) {
            wchar_t ver[256] = {};
            DWORD sz = sizeof(ver);
            RegQueryValueExW(hKey, L"DisplayVersion", NULL, NULL, (LPBYTE)ver, &sz);
            RegCloseKey(hKey);
            lua_pushstring(L, UnicodeToUtf8(ver).c_str());
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

// ── Install state persistence ─────────────────────────────────────────────

extern "C"
static int l_StateWrite(lua_State* L)
{
    const char* key   = luaL_checkstring(L, 1);
    const char* value = luaL_checkstring(L, 2);
    if (key == nullptr || value == nullptr || _strStateFile.empty()) {
        lua_pushboolean(L, false);
        return 1;
    }
    BOOL ok = WritePrivateProfileStringW(
        L"State",
        Utf82Unicode(key).c_str(),
        Utf82Unicode(value).c_str(),
        _strStateFile.c_str());
    lua_pushboolean(L, ok != 0);
    return 1;
}

extern "C"
static int l_StateRead(lua_State* L)
{
    const char* key = luaL_checkstring(L, 1);
    if (key == nullptr || _strStateFile.empty()) { lua_pushnil(L); return 1; }
    wchar_t buf[1024] = {};
    DWORD len = GetPrivateProfileStringW(
        L"State",
        Utf82Unicode(key).c_str(),
        L"",
        buf, (DWORD)std::size(buf),
        _strStateFile.c_str());
    if (len == 0) { lua_pushnil(L); return 1; }
    lua_pushstring(L, UnicodeToUtf8(buf).c_str());
    return 1;
}

extern "C"
static int l_CheckpointSet(lua_State* L)
{
    const char* phase = luaL_checkstring(L, 1);
    if (phase == nullptr || _strStateFile.empty()) {
        lua_pushboolean(L, false);
        return 1;
    }
    BOOL ok = WritePrivateProfileStringW(
        L"Checkpoint", L"Phase",
        Utf82Unicode(phase).c_str(),
        _strStateFile.c_str());
    lua_pushboolean(L, ok != 0);
    return 1;
}

extern "C"
static int l_CheckpointGet(lua_State* L)
{
    if (_strStateFile.empty()) { lua_pushnil(L); return 1; }
    wchar_t buf[256] = {};
    DWORD len = GetPrivateProfileStringW(
        L"Checkpoint", L"Phase", L"",
        buf, (DWORD)std::size(buf),
        _strStateFile.c_str());
    if (len == 0) { lua_pushnil(L); return 1; }
    lua_pushstring(L, UnicodeToUtf8(buf).c_str());
    return 1;
}

// Embedded base library loaded into every installer Lua state.
// Content mirrors install_base.lua (kept in sync manually).
static const char g_install_base_lua[] = R"LUA(
HRootKey = {
    HKEY_CLASSES_ROOT   = 0,
    HKEY_CURRENT_USER   = 1,
    HKEY_LOCAL_MACHINE  = 2,
    HKEY_USERS          = 3,
    HKEY_CURRENT_CONFIG = 4,
}
CSIDL = {
    DESKTOP=0, PROGRAMS=2, CONTROLS=3, PRINTERS=4, PERSONAL=5,
    FAVORITES=6, STARTUP=7, RECENT=8, SENDTO=9, STARTMENU=11,
    MYDOCUMENTS=5, MYMUSIC=13, MYVIDEO=14, DESKTOPDIRECTORY=16,
    DRIVES=17, NETWORK=18, NETHOOD=19, FONTS=20, TEMPLATES=21,
    COMMON_STARTMENU=22, COMMON_PROGRAMS=23, COMMON_STARTUP=24,
    COMMON_DESKTOPDIRECTORY=25, APPDATA=26, PRINTHOOD=27, PROGRAM_FILES=38,
}
function RunBatchFile(batchPath, waitSeconds)
    if not installx.FilePathExists(batchPath) then
        installx.LogPrint("Skip missing batch file: " .. batchPath)
        return false
    end
    local command = 'cmd.exe /S /C call "' .. batchPath .. '"'
    installx.LogPrint("RunBatchFile: " .. command)
    return installx.ProcessExecute(command, false, waitSeconds or 30)
end
function ErrorHint(msg)
    installx.LogPrint("[Error] " .. tostring(msg))
    installx.DuiText("errortiplab", tostring(msg))
    installx.DuiVisible("errortiplab", true)
end
)LUA";

static const luaL_Reg reglib[] = {
    {"DiskFreeSpace", l_DiskFreeSpace},
    {"DuiEnable", l_DuiEnable},
    {"DuiMessage", l_DuiMessage},
    {"DuiOptionSelect", l_DuiOptionSelect},
    {"DuiProgress", l_DuiProgress},
    {"DuiSetBkImage", l_DuiSetBkImage},
    {"DuiTabSelect", l_DuiTabSelect},
    {"DuiText", l_DuiText},
    {"DuiTextColor", l_DuiTextColor},
    {"DuiVisible", l_DuiVisible},
    {"DuiWindowPos", l_DuiWindowPos},

    {"FilePathChoose", l_FilePathChoose},
    {"FilePathCreateShortCut", l_FilePathCreateShortCut},
    {"FilePathCopy", l_FilePathCopy},
    {"FilePathDelete", l_FilePathDelete},
    {"FilePathExists", l_FilePathExists},
    {"FilePathGetSpecialLocation", l_FilePathGetSpecialLocation},
    {"FilePathMkdir", l_FilePathMakeDir},
    {"FilePathUnzip", l_FilePathUnzipAsync},

    {"LogPrint", l_LogPrint},

    {"ProcessExecute", l_ProcessExecute},
    {"ProcessKill", l_ProcessKill},

    {"RunAsAdmin", l_RunAsAdmin},

    {"RegGetValue", l_RegGetValue},
    {"RegSetValue", l_RegSetValue},
    {"RegDeleteValue", l_RegDeleteValue},
    {"RegDeleteKey", l_RegDeleteKey},

    {"SysACP", l_SysACP},
    {"SysEnvSet", l_SysEnvSet},
    {"SysGetArch", l_SysGetArch},
    {"SysGetInstalledSoftware", l_SysGetInstalledSoftware},
    {"SysIsWin10OrLater", l_SysIsWin10OrLater},
    {"SysNetworkAvailable", l_SysNetworkAvailable},
    {"SysPathAdd", l_SysPathAdd},

    {"StateRead", l_StateRead},
    {"StateWrite", l_StateWrite},
    {"CheckpointGet", l_CheckpointGet},
    {"CheckpointSet", l_CheckpointSet},

    {NULL, NULL}
};

int luaopen_reg(lua_State* L) 
{
    luaL_newlib(L, reglib);
    return 1;
}

//////////////////////////////////////////////////////////////////////////

lua_function_base::lua_function_base(lua_State* vm, const std::string& func)
    : m_vm(vm), m_func(-1)
{
    lua_getglobal(m_vm, func.c_str());
    if (!lua_isfunction(m_vm, -1)) {
        lua_pop(m_vm, 1);
    }
    else {
        m_func = luaL_ref(m_vm, LUA_REGISTRYINDEX);
    }
}

lua_function_base::lua_function_base(const lua_function_base& func)
    : m_vm(func.m_vm)
{
    lua_rawgeti(m_vm, LUA_REGISTRYINDEX, func.m_func);
    m_func = luaL_ref(m_vm, LUA_REGISTRYINDEX);
}

lua_function_base::~lua_function_base()
{
    luaL_unref(m_vm, LUA_REGISTRYINDEX, m_func);
}

lua_function_base& lua_function_base::operator=(const lua_function_base& func)
{
    if (this != &func) {
        m_vm = func.m_vm;
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, func.m_func);
        m_func = luaL_ref(m_vm, LUA_REGISTRYINDEX);
    }
    return *this;
}

void lua_function_base::push_value(lua_State* vm, int n)
{
    lua_pushinteger(vm, lua_Integer(n));
}

void lua_function_base::push_value(lua_State* vm, lua_Integer n)
{
    lua_pushinteger(vm, n);
}

void lua_function_base::push_value(lua_State* vm, lua_Number n)
{
    lua_pushnumber(vm, n);
}

void lua_function_base::push_value(lua_State* vm, bool b)
{
    lua_pushboolean(vm, b);
}

void lua_function_base::push_value(lua_State* vm, const std::string& s)
{
    lua_pushstring(vm, s.c_str());
}

void lua_function_base::call(int args, int results)
{
    // Push traceback handler before args/function
    lua_pushcfunction(m_vm, [](lua_State* L) -> int {
        luaL_traceback(L, L, lua_tostring(L, 1), 1);
        return 1;
    });
    // Move handler below function and args: stack is [handler, func, args...]
    lua_insert(m_vm, -(args + 2));

    int handlerIdx = lua_gettop(m_vm) - args - 1;
    int status = lua_pcall(m_vm, args, results, handlerIdx);
    lua_remove(m_vm, handlerIdx); // remove handler

    if (status != 0) {
        const char* err = lua_tostring(m_vm, -1);
        if (err) {
            APPLOG(Log::LOG_ERROR)("[Lua] %s\n", err);
            OutputDebugStringA(err);
            OutputDebugStringA("\n");
        }
        lua_pop(m_vm, 1);
    }
}

//////////////////////////////////////////////////////////////////////////

lua_base::lua_base()
{
    lua_ = luaL_newstate();
    luaL_openlibs(lua_);
    lua_pushlightuserdata(lua_, this);
    lua_setfield(lua_, LUA_REGISTRYINDEX, "installx.vm");
    luaL_requiref(lua_, "installx", luaopen_reg, 1);
    lua_pop(lua_, 1);

    // Load the embedded base library (HRootKey, CSIDL, RunBatchFile, ErrorHint)
    if (luaL_dostring(lua_, g_install_base_lua) != LUA_OK) {
        const char* err = lua_tostring(lua_, -1);
        OutputDebugStringA(err ? err : "installx base library load failed");
        lua_pop(lua_, 1);
    }
}

lua_base::~lua_base()
{
    if (lua_)
        lua_close(lua_);
}

int lua_base::load_file(const char* fullpath)
{
    struct stat st;
    if (::stat(fullpath, &st) < 0)
        return -1;
    int ret = luaL_dofile(lua_, fullpath);
    if (ret != 0)
        return -1;
    return 0;
}

int lua_base::load_string(const char* cstr)
{
    int ret = luaL_dostring(lua_, cstr);
    if (ret != 0) {
        const char* err = lua_tostring(lua_, -1);
        std::string msg = err ? err : "unknown error";
        lua_pop(lua_, 1);
        APPLOG(Log::LOG_ERROR)("[Lua] Script load error: %s\n", msg.c_str());
        OutputDebugStringA(("[Lua] " + msg + "\n").c_str());
        MessageBoxA(NULL, msg.c_str(), "Lua Script Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////

InstallLua::InstallLua(DuiLib::CPaintManagerUI* pPaingManger, const std::string& version)
{
    _pPaintManager = pPaingManger;
    _strVersion = version;

    // Derive a per-installer state file path from the window title.
    // Stored in %TEMP%\InstallXpress\<title>.state
    wchar_t tempDir[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, tempDir);
    std::wstring stateDir = std::wstring(tempDir) + L"InstallXpress\\";
    CreateDirectoryW(stateDir.c_str(), NULL);
    std::wstring title = Utf82Unicode(version);
    // Sanitize: replace all characters invalid in Windows file names with '_'
    for (wchar_t& c : title) {
        if (c == L'\\' || c == L'/' || c == L':' || c == L'*' || c == L'?' ||
            c == L'"'  || c == L'<' || c == L'>' || c == L'|')
            c = L'_';
    }
    // Trim trailing dots and spaces (also illegal in Windows file names)
    while (!title.empty() && (title.back() == L'.' || title.back() == L' '))
        title.pop_back();
    _strStateFile = stateDir + title + L".state";
}

InstallLua::~InstallLua()
{

}

void InstallLua::OnInitialize()
{
    lua_function<void> func(lua_, "OnInitialize");
    func();
}

void InstallLua::OnButtonClick(const std::string& strButtonName)
{
    lua_function<void> func(lua_, "OnButtonClick");
    func(strButtonName);
}

void InstallLua::OnSelChanged(const std::string& strButtonName, bool isSelected)
{
    lua_function<void> func(lua_, "OnSelChanged");
    func(strButtonName, isSelected);
}

void InstallLua::OnUnzipProgress(int nNotifyID, unsigned nFileNum, unsigned nCurFileIndex, unsigned long long i64TotalSize, unsigned long long iCurSize)
{
    lua_function<void> func(lua_, "OnUnzipProgress");
    func(nNotifyID, (lua_Integer)nFileNum, (lua_Integer)nCurFileIndex, (lua_Integer)i64TotalSize, (lua_Integer)iCurSize);
}

void InstallLua::ResetInstallPath(const std::string& strInstallPath)
{
    lua_function<void> func(lua_, "ResetInstallPath");
    func(strInstallPath);
}

void InstallLua::PostSetup()
{
    lua_function<void> func(lua_, "PostSetup");
    func();
}


std::string InstallLua::QueryByKey_String(const std::string& keyName)
{
    lua_function<std::string> func(lua_, "QueryByKey");
    return func(keyName);
}

bool InstallLua::QueryByKey_Bool(const std::string& keyName)
{
    lua_function<bool> func(lua_, "QueryByKey");
    return func(keyName);
}

void InstallLua::SetByKey_String(const std::string& keyName, const std::string& strValue)
{
    lua_function<void> func(lua_, "SetByKey");
    return func(keyName, strValue);
}
