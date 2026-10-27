-- Run from the repository root: lua tests/test_taijiopcsim_dcom.lua
-- The TaijiMPC and TaijiMPC6 Install.lua must leave HKLM as TaiJiOPCSim-InteractiveUser.reg does,
-- and their UnInstall.lua must take it away again.
local CLSID = "{A48A6241-A024-4F99-B105-5DF8CCEA66BA}"
local EXE = "C:\\TaijiControl\\TaiJiOPCSim\\bin\\TaiJiOPCSim.exe"

-- Key path -> {value name -> data}. Key and value names are case-insensitive, and
-- SOFTWARE\WOW6432Node\Classes is the same 32-bit classes view as SOFTWARE\Classes\WOW6432Node.
local registry, events, exists

local function keyOf(path)
    local key = path:lower()
    local alias = "software\\wow6432node\\classes\\"
    if key:sub(1, #alias) == alias then
        key = "software\\classes\\wow6432node\\" .. key:sub(#alias + 1)
    end
    return key
end

local function value(path, name)
    local values = registry[keyOf(path)]
    return values and values[name:lower()]
end

HRootKey = {HKEY_LOCAL_MACHINE = 2}
CSIDL = {COMMON_DESKTOPDIRECTORY = 25, COMMON_STARTMENU = 22}
installx = {
    FilePathExists = function(path) return exists[path] == true end,
    ProcessExecute = function(command) table.insert(events, "exec " .. command) return true end,
    RegSetValue = function(root, path, name, data)
        assert(root == HRootKey.HKEY_LOCAL_MACHINE, path)
        table.insert(events, "set " .. path)
        local key = keyOf(path)
        registry[key] = registry[key] or {}
        registry[key][name:lower()] = data
        return true
    end,
    RegDeleteKey = function(root, path, name)
        assert(root == HRootKey.HKEY_LOCAL_MACHINE, path)
        local doomed = keyOf(path .. "\\" .. name)
        for key in pairs(registry) do
            if key == doomed or key:sub(1, #doomed + 1) == doomed .. "\\" then
                registry[key] = nil
            end
        end
        return true
    end,
    RegDeleteValue = function() return true end,
    RegGetValue = function() return nil end,
    ProcessKill = function() return true end,
    FilePathGetSpecialLocation = function() return "C:\\Users\\Public\\Desktop" end,
    FilePathDelete = function() return true end,
    LogPrint = function() end,
}

local function reset(installed)
    registry, events, exists = {}, {}, {[EXE] = installed}
end

local CATEGORIES = "SOFTWARE\\Classes\\WOW6432Node\\CLSID\\" .. CLSID .. "\\Implemented Categories\\"
local projects = {"TaijiMPC", "TaijiMPC6"}
for _, project in ipairs(projects) do
    RegisterTaiJiOPCSim, PreSetup = nil, nil
    assert(loadfile("projects/" .. project .. "/Install/Install.lua"))()

    -- Installed: registered first, then the keys of TaiJiOPCSim-InteractiveUser.reg on top.
    reset(true)
    RegisterTaiJiOPCSim()
    assert(events[1] == "exec \"" .. EXE .. "\" -RegServer", project .. ": " .. tostring(events[1]))
    assert(value("SOFTWARE\\WOW6432Node\\Classes\\CLSID\\" .. CLSID, "AppID") == CLSID, project)
    assert(value("SOFTWARE\\Classes\\AppID\\" .. CLSID, "") == "TaiJiOPCSim", project)
    assert(value("SOFTWARE\\Classes\\AppID\\" .. CLSID, "RunAs") == "Interactive User", project)
    assert(value("SOFTWARE\\Classes\\AppID\\TaiJiOPCSim.exe", "AppID") == CLSID, project)
    -- AppID is a shared key: written to the 64-bit view, never under WOW6432Node.
    for key in pairs(registry) do
        assert(not (key:find("wow6432node", 1, true) and key:find("\\appid\\", 1, true)), project .. ": " .. key)
    end
    assert(value(CATEGORIES .. "{63D5F430-CFE4-11d1-B2C8-0060083BA1FB}", "") == "", project)
    assert(value(CATEGORIES .. "{63D5F432-CFE4-11d1-B2C8-0060083BA1FB}", "") == "", project)
    local written = 0
    for _ in pairs(registry) do written = written + 1 end
    -- CLSID, its Implemented Categories and the two category subkeys, the two AppID keys.
    assert(written == 6, project .. ": unexpected keys written: " .. written)

    -- Uninstall removes every one of them.
    assert(loadfile("projects/" .. project .. "/UnInstall/res/UnInstall.lua"))()
    PreSetup()
    local left = next(registry)
    assert(left == nil, project .. ": left behind by UnInstall.lua: " .. tostring(left))

    -- Not installed: no registration, no keys pointing at a missing exe.
    reset(false)
    RegisterTaiJiOPCSim()
    assert(#events == 0, project .. ": " .. tostring(events[1]))
end

print("TaiJiOPCSim DCOM identity: install and uninstall cases passed for " .. table.concat(projects, ", "))
