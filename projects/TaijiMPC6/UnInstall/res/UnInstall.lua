-- TaijiMPC UnInstall script.
-- HRootKey, CSIDL, RunBatchFile, ErrorHint are provided by the framework base library.
--
-- The native host (CUninstallMainFrame) drives the UI, then calls:
--   OnInitialize()  -- when the window is ready
--   PreSetup()      -- after the user confirms, on the worker thread
-- and afterwards recursively deletes the directory the running UnInstall.exe
-- lives in (e.g. <InstallPath>\TaiJiMPC5) and self-deletes it.
--
-- Therefore PreSetup() performs all product-specific cleanup (stopping
-- services, unregistering COM servers, removing registry keys / shortcuts and
-- deleting every *other* install sub-directory). It is the reverse of
-- projects/TaijiMPC/Install/Install.lua.

local DEFAULT_INSTALL_PATH = "C:\\TaijiControl"

local _dirCompany = DEFAULT_INSTALL_PATH
-- The directory that holds the running UnInstall.exe; the native host removes it.
local _dirExeHome = _dirCompany .. "\\TaiJiMPC5"

local _strResourceCN = {
    SETUP        = "泰极MPC卸载程序",
    UNINSTALLING = "正在卸载",
}

local _strResourceEN = {
    SETUP        = "Tai-Ji MPC Uninstall",
    UNINSTALLING = "Uninstalling",
}

local _strResource = _strResourceCN

local function resetInstallPath(installPath)
    if installPath == nil or installPath == "" then
        installPath = DEFAULT_INSTALL_PATH
    end
    _dirCompany = installPath
    _dirExeHome = _dirCompany .. "\\TaiJiMPC5"
end

local function deleteIfExists(path)
    if path ~= nil and path ~= "" and installx.FilePathExists(path) then
        installx.LogPrint("Delete: " .. path)
        return installx.FilePathDelete(path)
    end
    return true
end

-- Run an uninstall batch file only if it exists (safe no-op otherwise).
local function runUninstallBatch(batchPath, waitSec)
    if installx.FilePathExists(batchPath) then
        installx.LogPrint("RunUninstallBatch: " .. batchPath)
        RunBatchFile(batchPath, waitSec)
    end
end

local function normalizePath(path)
    local value = tostring(path or "")
    value = value:gsub("/", "\\")
    value = value:gsub("\\+$", "")
    return value:lower()
end

-- Remove the given entries from the system PATH (reverse of Install SysPathAdd).
local function removePathEntries(...)
    local envKey = "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"
    local pathValue = installx.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, envKey, "Path")
    if pathValue == nil or pathValue == "" then
        return
    end

    local removals = {}
    for _, path in ipairs({ ... }) do
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
    resetInstallPath(installx.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl", "InstallPath"))

    local acp = installx.SysACP()
    if acp == 936 then
        _strResource = _strResourceCN
    else
        _strResource = _strResourceEN
    end

    installx.LogPrint("UnInstall OnInitialize, dirCompany: " .. _dirCompany)
end

function PreSetup()
    installx.LogPrint("UnInstall PreSetup begin")

    -- 1. Stop running applications.
    installx.ProcessKill("TaiJiMPC.exe")
    installx.ProcessKill("TaiJiOPCSim.exe")

    -- 2. Stop and remove the HostVM Windows service (installed in Install.lua PostSetup).
    installx.ProcessExecute("\"" .. _dirCompany .. "\\HostVM\\HostVM.exe\" service stop HostVM", true, 15)
    installx.ProcessExecute("\"" .. _dirCompany .. "\\HostVM\\HostVM.exe\" service remove HostVM", true, 15)
    installx.ProcessExecute("\"" .. _dirCompany .. "\\HostVM\\HostVM.exe\" --uninstall HostVM", true, 15)
    installx.ProcessKill("HostVM.exe")

    -- 3. Unregister the OPC simulator COM server (registered with -RegServer).
    if installx.FilePathExists(_dirCompany .. "\\TaiJiOPCSim\\bin\\TaiJiOPCSim.exe") then
        installx.ProcessExecute("\"" .. _dirCompany .. "\\TaiJiOPCSim\\bin\\TaiJiOPCSim.exe\" -UnRegServer", false, 10)
    end

    -- 4. Uninstall the HASP runtime driver (reverse of haspdinst.exe -install).
    if installx.FilePathExists(_dirCompany .. "\\Common\\hasp\\haspdinst.exe") then
        installx.ProcessExecute("\"" .. _dirCompany .. "\\Common\\hasp\\haspdinst.exe\" -r -kp -nomsg", true, 30)
    end

    -- 5. Run uninstall counterparts of the install_*.bat scripts, when present.
    runUninstallBatch(_dirCompany .. "\\TaiJiDataSvc\\uninstall_TaiJiDataSvc.bat", 30)
    runUninstallBatch(_dirCompany .. "\\TaiJiDataClient\\uninstall_TaiJiDataClient.bat", 30)
    runUninstallBatch(_dirCompany .. "\\TaijiPYSim\\uninstall_taijiopcsim.bat", 30)
    runUninstallBatch(_dirCompany .. "\\HostVM\\uninstall_python_env.bat", 10)

    -- 6. Remove the custom OPC enumerator COM class created in Install.lua PostSetup.
    installx.RegDeleteKey(HRootKey.HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Classes\\WOW6432Node\\CLSID", "{A48A6241-A024-4f99-B105-5DF8CCEA66BA}")

    -- 7. Remove all product registry trees (InstallPath / TaiJiMPC5 / PythonEnv).
    installx.RegDeleteKey(HRootKey.HKEY_LOCAL_MACHINE, "Software", "TaijiControl")
    installx.RegDeleteKey(HRootKey.HKEY_LOCAL_MACHINE, "Software\\WOW6432Node", "TaijiControl")

    -- 8. Remove autostart entry and the Add/Remove-Programs uninstall entry.
    installx.RegDeleteValue(HRootKey.HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "TaijiMPC")
    installx.RegDeleteKey(HRootKey.HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall", "TaiJiMPC5")

    -- 9. Remove the Python environment changes (reverse of _FinishInstall).
    removePathEntries(_dirCompany .. "\\WinPy313\\python", _dirCompany .. "\\WinPy313\\python\\Scripts")
    installx.RegDeleteValue(HRootKey.HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", "PYTHONHOME")

    -- 10. Remove desktop and start-menu shortcuts.
    local desktopDir = installx.FilePathGetSpecialLocation(CSIDL.COMMON_DESKTOPDIRECTORY)
    deleteIfExists(desktopDir .. "\\TaiJiMPC5.lnk")
    deleteIfExists(desktopDir .. "\\TaiJiMPC6.lnk")

    local startMenuDir = installx.FilePathGetSpecialLocation(CSIDL.COMMON_STARTMENU)
    deleteIfExists(startMenuDir .. "\\Programs\\TaijiControl")

    -- 11. Delete every install sub-directory except the one the running
    --      UnInstall.exe lives in (_dirExeHome); the native host removes that one.
    deleteIfExists(_dirCompany .. "\\TaiJiMPC6")
    deleteIfExists(_dirCompany .. "\\WinPy313")
    deleteIfExists(_dirCompany .. "\\Win32")
    deleteIfExists(_dirCompany .. "\\TaiJiDataSvc")
    deleteIfExists(_dirCompany .. "\\TaiJiDataClient")
    deleteIfExists(_dirCompany .. "\\TaijiPYSim")
    deleteIfExists(_dirCompany .. "\\TaiJiOPCSim")
    deleteIfExists(_dirCompany .. "\\HostVM")
    deleteIfExists(_dirCompany .. "\\Common")

    installx.LogPrint("UnInstall PreSetup done, exe home left for native cleanup: " .. _dirExeHome)
end

function PostSetup()
end

function OnButtonClick(btnName)
end

function QueryByKey(keyName)
    if keyName == "InstallPath" then
        return _dirCompany
    elseif keyName == "AppHome" then
        return _dirExeHome
    end
end
