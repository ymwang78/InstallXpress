-- HRootKey, CSIDL, RunBatchFile, ErrorHint are provided by the framework base library.

local DEFAULT_INSTALL_PATH = "C:\\ZJU"

local dirCompany = DEFAULT_INSTALL_PATH
local dirExeHomeDir = dirCompany .. "\\Rto"
local dirExeFullPath = dirExeHomeDir .. "\\xRto.exe"

local function resetInstallPath(installPath)
    if installPath == nil or installPath == "" then
        installPath = DEFAULT_INSTALL_PATH
    end
    dirCompany = installPath
    dirExeHomeDir = dirCompany .. "\\Rto"
    dirExeFullPath = dirExeHomeDir .. "\\xRto.exe"
end

local function deleteIfExists(path)
    if path ~= nil and path ~= "" and installx.FilePathExists(path) then
        installx.LogPrint("Delete: " .. path)
        return installx.FilePathDelete(path)
    end
    installx.LogPrint("Skip missing: " .. tostring(path))
    return true
end

local function normalizePath(path)
    local value = tostring(path or "")
    value = value:gsub("/", "\\")
    value = value:gsub("\\+$", "")
    return value:lower()
end

local function removePathEntries(...)
    local envKey = "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"
    local pathValue = installx.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, envKey, "Path")
    if pathValue == nil or pathValue == "" then
        return
    end

    local removals = {}
    for _, path in ipairs({...}) do
        removals[normalizePath(path)] = true
    end

    local kept = {}
    for entry in tostring(pathValue):gmatch("([^;]+)") do
        if entry ~= "" and not removals[normalizePath(entry)] then
            table.insert(kept, entry)
        end
    end

    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, envKey, "Path", table.concat(kept, ";"))
end

function OnInitialize()
    resetInstallPath(installx.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\ZJU", "InstallPath"))
    installx.LogPrint("RTO uninstall path: " .. dirCompany)
end

function PreSetup()
    installx.LogPrint("Start RTO uninstall cleanup")

    installx.ProcessKill("xRto.exe")

    installx.RegDeleteValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "Rto")
    installx.RegDeleteKey(HRootKey.HKEY_LOCAL_MACHINE, "Software\\ZJU", "Rto")
    installx.RegDeleteKey(HRootKey.HKEY_LOCAL_MACHINE, "Software\\ZJU", "PythonEnv")
    installx.RegDeleteValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\ZJU", "InstallPath")
    installx.RegDeleteKey(HRootKey.HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall", "Rto")

    removePathEntries(dirCompany .. "\\WinPy312\\python", dirCompany .. "\\WinPy312\\python\\Scripts")
    installx.RegDeleteValue(
        HRootKey.HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",
        "PYTHONHOME")

    local desktopDir = installx.FilePathGetSpecialLocation(CSIDL.COMMON_DESKTOPDIRECTORY)
    deleteIfExists(desktopDir .. "\\Rto.lnk")

    local startMenuDir = installx.FilePathGetSpecialLocation(CSIDL.COMMON_STARTMENU)
    deleteIfExists(startMenuDir .. "\\Programs\\ZJU\\Rto.lnk")
    deleteIfExists(startMenuDir .. "\\Programs\\ZJU")

    -- Do not delete dirExeHomeDir here because UnInstall.exe is running from it.
    -- The native host removes that directory after this scripted cleanup.
    deleteIfExists(dirCompany .. "\\WinPy312")
    deleteIfExists(dirCompany .. "\\Common")
    deleteIfExists(dirCompany .. "\\HttpServer")
    deleteIfExists(dirCompany .. "\\Script")
    deleteIfExists(dirCompany .. "\\Solver")
    deleteIfExists(dirCompany .. "\\UnitModel")
end

function PostSetup()
end

function OnButtonClick(btnName)
end

function QueryByKey(keyName)
    if keyName == "InstallPath" then
        return dirCompany
    elseif keyName == "AppPath" then
        return dirExeFullPath
    elseif keyName == "AppHome" then
        return dirExeHomeDir
    end
end
