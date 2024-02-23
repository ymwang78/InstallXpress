#pragma once
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

class  lua_base
{
protected:
    lua_State* lua_;

    const char* str_; //resource string

    lua_base();

    ~lua_base();

public:

    int load_file(const char* fullpath);

    int load_string(const char* cstr);
};


template <typename T>
struct value_extractor
{
};

template <>
struct value_extractor<int>
{
    static int get(lua_State* vm)
    {
        lua_Integer val = lua_tointeger(vm, -1);
        lua_pop(vm, 1);
        return (int)val;
    }
};

template <>
struct value_extractor<lua_Integer>
{
    static lua_Integer get(lua_State* vm)
    {
        lua_Integer val = lua_tointeger(vm, -1);
        lua_pop(vm, 1);
        return val;
    }
};

template <>
struct value_extractor<lua_Number>
{
    static lua_Number get(lua_State* vm)
    {
        lua_Number val = lua_tonumber(vm, -1);
        lua_pop(vm, 1);
        return val;
    }
};

template <>
struct value_extractor<bool>
{
    static bool get(lua_State* vm)
    {
        bool val = lua_toboolean(vm, -1);
        lua_pop(vm, 1);
        return val;
    }
};

template <>
struct value_extractor<std::string>
{
    static std::string get(lua_State* vm)
    {
        std::string val = lua_tostring(vm, -1);
        lua_pop(vm, 1);
        return val;
    }
};


// the base function wrapper class
class lua_function_base
{
public:
    lua_function_base(lua_State* vm, const std::string& func);

    lua_function_base(const lua_function_base& func);

    ~lua_function_base();

    lua_function_base& operator=(const lua_function_base& func);

protected:
    lua_State* m_vm;
    int m_func;

    static void push_value(lua_State* vm, int n);

    static void push_value(lua_State* vm, lua_Integer n);

    static void push_value(lua_State* vm, lua_Number n);

    static void push_value(lua_State* vm, bool b);

    static void push_value(lua_State* vm, const std::string& s);

    void call(int args, int results);
};


template <typename Ret>
class lua_function : public lua_function_base
{
public:
    lua_function(lua_State* vm, const std::string& func)
        : lua_function_base(vm, func)
    {
    }

    Ret operator()()
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        call(0, 1);
        return value_extractor<Ret>::get(m_vm);
    }

    template <typename T1>
    Ret operator()(const T1& p1)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        call(1, 1);
        return value_extractor<Ret>::get(m_vm);
    }

    template <typename T1, typename T2>
    Ret operator()(const T1& p1, const T2& p2)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        push_value(m_vm, p2);
        call(2, 1);
        return value_extractor<Ret>::get(m_vm);
    }

    template <typename T1, typename T2, typename T3>
    Ret operator()(const T1& p1, const T2& p2, const T3& p3)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        push_value(m_vm, p2);
        push_value(m_vm, p3);
        call(3, 1);
        return value_extractor<Ret>::get(m_vm);
    }

    template <typename T1, typename T2, typename T3, typename T4>
    Ret operator()(const T1& p1, const T2& p2, const T3& p3, const T4& p4)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        push_value(m_vm, p2);
        push_value(m_vm, p3);
        push_value(m_vm, p4);
        call(4, 1);
        return value_extractor<Ret>::get(m_vm);
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5>
    Ret operator()(const T1& p1, const T2& p2, const T3& p3, const T4& p4, const T5& p5)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        push_value(m_vm, p2);
        push_value(m_vm, p3);
        push_value(m_vm, p4);
        push_value(m_vm, p5);
        call(5, 1);
        return value_extractor<Ret>::get(m_vm);
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    Ret operator()(const T1& p1, const T2& p2, const T3& p3, const T4& p4, const T5& p5, const T6& p6)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        push_value(m_vm, p2);
        push_value(m_vm, p3);
        push_value(m_vm, p4);
        push_value(m_vm, p5);
        push_value(m_vm, p6);
        call(6, 1);
        return value_extractor<Ret>::get(m_vm);
    }


    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    Ret operator()(const T1& p1, const T2& p2, const T3& p3, const T4& p4, const T5& p5, const T6& p6, const T7& p7)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        push_value(m_vm, p2);
        push_value(m_vm, p3);
        push_value(m_vm, p4);
        push_value(m_vm, p5);
        push_value(m_vm, p6);
        push_value(m_vm, p7);
        call(7, 1);
        return value_extractor<Ret>::get(m_vm);
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    Ret operator()(const T1& p1, const T2& p2, const T3& p3, const T4& p4, const T5& p5, const T6& p6, const T7& p7, const T8& p8)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        push_value(m_vm, p2);
        push_value(m_vm, p3);
        push_value(m_vm, p4);
        push_value(m_vm, p5);
        push_value(m_vm, p6);
        push_value(m_vm, p7);
        push_value(m_vm, p8);
        call(8, 1);
        return value_extractor<Ret>::get(m_vm);
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    Ret operator()(const T1& p1, const T2& p2, const T3& p3, const T4& p4, const T5& p5, const T6& p6, const T7& p7, const T8& p8, const T9& p9)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        push_value(m_vm, p2);
        push_value(m_vm, p3);
        push_value(m_vm, p4);
        push_value(m_vm, p5);
        push_value(m_vm, p6);
        push_value(m_vm, p7);
        push_value(m_vm, p8);
        push_value(m_vm, p9);
        call(9, 1);
        return value_extractor<Ret>::get(m_vm);
    }

};

template <>
class lua_function<void> : public lua_function_base
{
public:
    lua_function(lua_State* vm, const std::string& func)
        : lua_function_base(vm, func)
    {
    }

    void operator()()
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        call(0, 0);
    }

    template <typename T1>
    void operator()(const T1& p1)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        call(1, 0);
    }

    template <typename T1, typename T2>
    void operator()(const T1& p1, const T2& p2)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        push_value(m_vm, p2);
        call(2, 0);
    }

    template <typename T1, typename T2, typename T3>
    void operator()(const T1& p1, const T2& p2, const T3& p3)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        push_value(m_vm, p2);
        push_value(m_vm, p3);
        call(3, 0);
    }


    template <typename T1, typename T2, typename T3, typename T4>
    void operator()(const T1& p1, const T2& p2, const T3& p3, const T4& p4)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        push_value(m_vm, p2);
        push_value(m_vm, p3);
        push_value(m_vm, p4);
        call(4, 1);
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5>
    void operator()(const T1& p1, const T2& p2, const T3& p3, const T4& p4, const T5& p5)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        push_value(m_vm, p2);
        push_value(m_vm, p3);
        push_value(m_vm, p4);
        push_value(m_vm, p5);
        call(5, 1);
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void operator()(const T1& p1, const T2& p2, const T3& p3, const T4& p4, const T5& p5, const T6& p6)
    {
        lua_rawgeti(m_vm, LUA_REGISTRYINDEX, m_func);
        push_value(m_vm, p1);
        push_value(m_vm, p2);
        push_value(m_vm, p3);
        push_value(m_vm, p4);
        push_value(m_vm, p5);
        push_value(m_vm, p6);
        call(6, 1);
    }
};

/////////////////////////////////////////////////////////////
#include <DuiLib/UIlib.h>
#include <DuiLib/Core/UIManager.h>

class InstallLua : public lua_base
{

public:
    InstallLua(DuiLib::CPaintManagerUI* pPaingManger, const std::string& version);

    ~InstallLua();

    void OnInitialize();

    void OnButtonClick(const std::string& strButtonName);

    void OnSelChanged(const std::string& strButtonName, bool isSelected);

    void ResetInstallPath(const std::string& strInstallPath);

    void PostSetup();

    std::string QueryByKey_String(const std::string& keyName);

    bool QueryByKey_Bool(const std::string& keyName);

    void SetByKey_String(const std::string& keyName, const std::string& strValue);
};
