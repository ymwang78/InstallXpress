-- Run from the repository root: lua tests/test_hasp_runtime.lua
local installed, commands
installx = {
    RegGetValue = function(root, path, key)
        assert(root == HRootKey.HKEY_LOCAL_MACHINE)
        assert(path == "SOFTWARE\\Aladdin Knowledge Systems\\HASP\\Driver\\Installer")
        assert(key == "DrvPkgVersion")
        return installed
    end,
    ProcessExecute = function(command) table.insert(commands, command) end,
    LogPrint = function() end,
}
HRootKey = {HKEY_LOCAL_MACHINE = 0x80000002}
local haspdinst = "\"C:\\TaijiControl\\Common\\hasp\\haspdinst.exe\" -install -nomsg"
-- DrvPkgVersion -> whether haspdinst runs. The 8.x/9.x/10.31 rows were skipped or inverted by the
-- old string comparison against "8.31".
local cases = {
    {nil, true},
    {"", true},
    {"8.31", true},
    {"8.41", true},
    {"9.12", true},
    {"10.21", true},
    {"10.31", true},
    {"10", true},
    {"10.33", false},
    {"10.33.0", false},
    {"10.33.1", false},
    {"10.40", false},
    {"10.100", false},
    {"11.0", false},
    {"10.3x", true},
    {"10..33", true},
    {"10.33.", true},
    {" 10.33", true},
    {1033, true},
}
local count = 0
for _, script in ipairs({"projects/TaijiMPC/Install/Install.lua", "projects/TaijiMPC6/Install/Install.lua"}) do
    InstallHaspRuntime = nil
    assert(loadfile(script))()
    for _, case in ipairs(cases) do
        installed, commands = case[1], {}
        InstallHaspRuntime()
        local label = script .. " " .. tostring(case[1])
        if case[2] then
            assert(#commands == 1 and commands[1] == haspdinst, label)
        else
            assert(#commands == 0, label)
        end
        count = count + 1
    end
end
print("HASP run-time installer: " .. count .. " cases passed")
