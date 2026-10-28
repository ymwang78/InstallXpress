-- Run from the repository root: lua tests/test_icu_fallback.lua
-- InstallIcuFallback() copies Common\icu's icu.dll and icuuc.dll into TaiJiMPC6\ only when the
-- system directory has no icu.dll.
local SYSTEM = "C:\\Windows\\System32"
local ROOT = "C:\\Test path\\中文"
local CLIENT = ROOT .. "\\TaiJiMPC6\\"
local FALLBACK = ROOT .. "\\Common\\icu\\"
local fs, copies, errors, systemDir, readOnly

installx = {
    FilePathGetSpecialLocation = function(id)
        assert(id == 0x25, "CSIDL_SYSTEM expected, got " .. tostring(id))
        return systemDir
    end,
    FilePathExists = function(path) return fs[path] ~= nil end,
    FilePathCopy = function(source, destination, failIfExists)
        assert(failIfExists == false, "the archive's icuuc.dll must be overwritten")
        if not fs[source] then return false, 2 end
        if readOnly[destination] and fs[destination] then return false, 5 end
        fs[destination] = fs[source]
        table.insert(copies, destination)
        return true, 0
    end,
    LogPrint = function() end,
}
ErrorHint = function(message) table.insert(errors, message) end
assert(loadfile("projects/TaijiMPC6/Install/Install.lua"))()
ResetInstallPath(ROOT)

local function reset(systemHasIcu)
    systemDir, copies, errors, readOnly = SYSTEM, {}, {}, {}
    fs = {
        [FALLBACK .. "icu.dll"] = "fallback icu",
        [FALLBACK .. "icuuc.dll"] = "fallback icuuc",
        [CLIENT .. "icuuc.dll"] = "archive icuuc",
    }
    if systemHasIcu then fs[SYSTEM .. "\\icu.dll"] = "system icu" end
end
local count = 0
local function check(expected, copied)
    assert(InstallIcuFallback() == expected, "case " .. count)
    assert(#copies == copied, "copy count " .. count .. ": " .. #copies)
    assert((#errors > 0) == not expected, "error hint " .. count)
    count = count + 1
end

-- Windows 10 1903 and later: the client keeps using the system icu.dll.
reset(true); check(true, 0)
assert(fs[CLIENT .. "icu.dll"] == nil and fs[CLIENT .. "icuuc.dll"] == "archive icuuc")

-- No system icu.dll: both files of the pair land next to TaiJiMPC.exe.
reset(false); check(true, 2)
assert(fs[CLIENT .. "icu.dll"] == "fallback icu" and fs[CLIENT .. "icuuc.dll"] == "fallback icuuc")

-- The system directory cannot be determined: deploy rather than leave the client unable to start.
reset(true); systemDir = nil; check(true, 2)

-- A read-only icu.dll put there by hand for an earlier version stays and setup goes on.
reset(false); fs[CLIENT .. "icu.dll"] = "manual icu"; readOnly[CLIENT .. "icu.dll"] = true; check(true, 1)
assert(fs[CLIENT .. "icu.dll"] == "manual icu" and fs[CLIENT .. "icuuc.dll"] == "fallback icuuc")

-- The fallback pair is missing from the package: setup fails instead of leaving no icu.dll.
reset(false); fs[FALLBACK .. "icu.dll"] = nil; check(false, 0)
assert(fs[CLIENT .. "icu.dll"] == nil)
reset(false); fs[FALLBACK .. "icuuc.dll"] = nil; fs[CLIENT .. "icuuc.dll"] = nil; check(false, 1)

print("ICU fallback installer: " .. count .. " cases passed")
