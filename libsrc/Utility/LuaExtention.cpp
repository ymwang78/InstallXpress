#include "stdafx.h"
#include <windows.h>
#include "TypeConvertUtil.h"
#include "LuaExtention.h"
#include <ShlObj_core.h>
#include <sstream> 
#include <process.h>
#include <shellapi.h>
#include <TlHelp32.h>

#define DEBUG_LUAEXT

static DuiLib::CPaintManagerUI* _pPaintManager = NULL;

static std::string _strVersion;

extern "C"
BOOL KillProcess(const wchar_t* szProcessName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return FALSE;
    PROCESSENTRY32 pe32 = { 0 };
    pe32.dwSize = sizeof(pe32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return FALSE;
    }
    do {
        if (_tcsicmp(pe32.szExeFile, szProcessName) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
            if (hProcess) {
                TerminateProcess(hProcess, 0);//关闭进程;
                CloseHandle(hProcess);
            }
        }
    } while (Process32Next(hSnapshot, &pe32));
    
    CloseHandle(hSnapshot);
    return TRUE;
}

extern "C"
BOOL FindProcess(const wchar_t* szProcessName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    PROCESSENTRY32 pe = { sizeof(pe) };
    BOOL bRet = Process32First(hSnapshot, &pe);
    while (bRet)
    {
        if (wcscmp(pe.szExeFile, szProcessName) == 0)
        {
            CloseHandle(hSnapshot);
            return TRUE;
        }
        bRet = Process32Next(hSnapshot, &pe);
    }
    CloseHandle(hSnapshot);
    return FALSE;
}

extern "C"
void ExecuteHiddenCommand(const wchar_t* cmd)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;  // 设置窗口隐藏

    ZeroMemory(&pi, sizeof(pi));

    // 创建隐藏的进程
    if (CreateProcess(NULL,   // 没有模块名（使用命令行）
        (LPTSTR)cmd,            // 命令行
        NULL,                  // 进程句柄不可继承
        NULL,                  // 线程句柄不可继承
        FALSE,                 // 设置句柄继承为 FALSE
        CREATE_NO_WINDOW,      // 不创建窗口
        NULL,                  // 使用父进程的环境块
        NULL,                  // 使用父进程的启动目录 
        &si,                   // 指向 STARTUPINFO 的指针
        &pi)                   // 指向 PROCESS_INFORMATION 的指针
        )
    {
        // 等待直到子进程退出。
        WaitForSingleObject(pi.hProcess, INFINITE);
        // 关闭进程和线程句柄
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

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

extern "C"
static int l_DuiGetOption(lua_State* L)
{
    const char* btnName = luaL_checkstring(L, 1);
    if (btnName == nullptr || _pPaintManager == 0) {
        lua_pushboolean(L, false);
        return 1;
    }
    DuiLib::COptionUI* pRunPiuPiu = static_cast<DuiLib::COptionUI*>(_pPaintManager->FindControl(Utf82Unicode(btnName).c_str()));
    if (pRunPiuPiu && pRunPiuPiu->IsSelected()) {
        lua_pushboolean(L, true);
        return 1;
    }
    lua_pushboolean(L, false);
    return 1;
}

extern "C"
static int l_LogPrint(lua_State * L) 
{
    int top = lua_gettop(L);
    for (int i = 1; i <= top; ++i) {
        OutputDebugStringA(lua_tostring(L, i));
    }
    OutputDebugStringA("\n");
    return 0;
}

extern "C"
static int l_RunCommand(lua_State * L) 
{
    const char* cmd = luaL_checkstring(L, 1);

#ifdef DEBUG_LUAEXT
    OutputDebugStringA("RunCommand");
    OutputDebugStringA(cmd);
#endif

    if (cmd == nullptr) {
        lua_pushboolean(L, false);
        return 1;
    }

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (!CreateProcessA(NULL, (LPSTR)cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        lua_pushboolean(L, false);
        return 1;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    lua_pushboolean(L, true);
    return 1;
}

extern "C"
static int l_RunShell(lua_State * L) 
{
    const char* cmd = luaL_checkstring(L, 1);

#ifdef DEBUG_LUAEXT
    OutputDebugStringA("RunShell");
    OutputDebugStringA(cmd);
#endif

    if (cmd == nullptr) {
        lua_pushboolean(L, false);
        return 1;
    }

    ExecuteHiddenCommand(Utf82Unicode(cmd).c_str());
    lua_pushboolean(L, true);
    return 1;
}

extern "C"
static int l_KillProcess(lua_State * L)
{
    const char* processName = luaL_checkstring(L, 1);
#ifdef DEBUG_LUAEXT
    OutputDebugStringA("KillProcess");
    OutputDebugStringA(processName);
#endif
    BOOL ret = KillProcess(Utf82Unicode(processName).c_str());
    lua_pushboolean(L, ret);
    return 1;
}

extern "C"
static int l_GetVersion(lua_State * L) 
{
    lua_pushstring(L, _strVersion.c_str());
    return 1;
}

extern "C"
static int l_GetSpecialFolderLocation(lua_State * L) 
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
static int l_CreateShortCut(lua_State * L)
{
    const char* lnk = luaL_checkstring(L, 1);
    const char* target = luaL_checkstring(L, 2);
    const char* workdir = luaL_checkstring(L, 3);
    const char* description = luaL_checkstring(L, 4);
    //const char* icon = luaL_checkstring(L, 5);
    //const char* hotkey = luaL_checkstring(L, 6);
    //const char* showcmd = luaL_checkstring(L, 7);

#ifdef DEBUG_LUAEXT
    OutputDebugStringA("CreateShortCut");
    OutputDebugStringA(lnk);
#endif

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
    //plink->SetIconLocation(Utf82Unicode(icon).c_str(), 0);
    //plink->SetHotkey((WORD)atoi(hotkey));
    //plink->SetShowCmd(atoi(showcmd));

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

extern "C"
static int l_CreateDirectory(lua_State * L) 
{
    const char* path = luaL_checkstring(L, 1);
#ifdef DEBUG_LUAEXT
    OutputDebugStringA("CreateDirectory");
    OutputDebugStringA(path);
#endif

    if (path == nullptr) {
        lua_pushboolean(L, false);
        return 1;
    }

    if (CreateDirectory(Utf82Unicode(path).c_str(), NULL)) {
        lua_pushboolean(L, true);
        return 1;
    }

    lua_pushboolean(L, false);
    return 1;
}

extern "C"
static int l_DeleteFile(lua_State * L)
{
    const char* path = luaL_checkstring(L, 1);
#ifdef DEBUG_LUAEXT
    OutputDebugStringA("DeleteFile");
    OutputDebugStringA(path);
#endif
    if (path == nullptr) {
        lua_pushboolean(L, false);
        return 1;
    }

    if (DeleteFile(Utf82Unicode(path).c_str())) {
        lua_pushboolean(L, true);
        return 1;
    }

    lua_pushboolean(L, false);
    return 1;
}

extern "C"
static int l_DeleteDirectory(lua_State * L) 
{
    const char* path = luaL_checkstring(L, 1);
#ifdef DEBUG_LUAEXT
    OutputDebugStringA("DeleteDirectory");
    OutputDebugStringA(path);
#endif
    if (path == nullptr) {
        lua_pushboolean(L, false);
        return 1;
    }

    if (RemoveDirectory(Utf82Unicode(path).c_str())) {
        lua_pushboolean(L, true);
        return 1;
    }

    lua_pushboolean(L, false);
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

#ifdef DEBUG_LUAEXT
    OutputDebugStringA("RegSetValue");
    OutputDebugStringA(path);
    OutputDebugStringA(key);
#endif

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

#ifdef DEBUG_LUAEXT
    OutputDebugStringA("RegDeleteValue");
    OutputDebugStringA(path);
    OutputDebugStringA(key);
#endif
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

#ifdef DEBUG_LUAEXT
    OutputDebugStringA("RegDeleteKey");
    OutputDebugStringA(path);
    if (key)
        OutputDebugStringA(key);
#endif
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

static const luaL_Reg reglib[] = {
    {"DuiGetOption", l_DuiGetOption},
    {"RunCommand", l_RunCommand},
    {"RunShell", l_RunShell},
    {"KillProcess", l_KillProcess},
    {"GetVersion", l_GetVersion},
    {"GetSpecialFolderLocation", l_GetSpecialFolderLocation},
    {"CreateShortCut", l_CreateShortCut},
    {"CreateDirectory", l_CreateDirectory},
    {"DeleteFile", l_DeleteFile},
    {"DeleteDirectory", l_DeleteDirectory},
    {"RegGetValue", l_RegGetValue},
    {"RegSetValue", l_RegSetValue},
    {"RegDeleteValue", l_RegDeleteValue},
    {"RegDeleteKey", l_RegDeleteKey},
    {"LogPrint", l_LogPrint},
    // 添加其他函数
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
    int status = lua_pcall(m_vm, args, results, 0);
    if (status != 0) {
        OutputDebugStringA(luas_dumpstack(m_vm, "load_string").c_str());
        std::string error = lua_tostring(m_vm, -1);
        lua_pop(m_vm, 1);
    }
}

//////////////////////////////////////////////////////////////////////////

lua_base::lua_base()
{
    lua_ = luaL_newstate();
    luaL_openlibs(lua_);
    lua_pushlightuserdata(lua_, this);
    lua_setfield(lua_, LUA_REGISTRYINDEX, "install.vm");
    luaL_requiref(lua_, "install", luaopen_reg, 1);
    lua_pop(lua_, 1);
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
        OutputDebugStringA(luas_dumpstack(lua_, "load_string").c_str());
        return -1;
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//    const std::wstring& ver = CMainFrame::GetInstance()->DirUtility().Version();

InstallLua::InstallLua(DuiLib::CPaintManagerUI* pPaingManger, const std::string& version)
{
    _pPaintManager = pPaingManger;
    _strVersion = version;
}

InstallLua::~InstallLua()
{

}

void InstallLua::Initialize()
{
    lua_function<void> func(lua_, "Initialize");
    func();
}

void InstallLua::ResetInstallPath(const std::string& strInstallPath)
{
    lua_function<void> func(lua_, "ResetInstallPath");
    func(strInstallPath);
}

void InstallLua::PreSetup()
{
    lua_function<void> func(lua_, "PreSetup");
    func();
}

void InstallLua::PostSetup()
{
    lua_function<void> func(lua_, "PostSetup");
    func();
}

void InstallLua::OnButtonClick(const std::string& strButtonName)
{
    lua_function<void> func(lua_, "OnButtonClick");
    func(strButtonName);
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
