-- HRootKey, CSIDL, RunBatchFile, ErrorHint are provided by the framework base library.

-- Resource ID constants (mirrors resource.h)
local RES = {
    LICENSE  = 133,  -- IDR_REGCONTENT1
    SOFT1    = 134,  -- IDR_INSTALLSOFT1 (Win32.7z)
    SOFT2    = 139,  -- IDR_INSTALLSOFT2 (WinPy313.7z)
    SOFT3    = 140,  -- IDR_INSTALLSOFT3 (TaiJiDataSvc.7z)
    BACKGROUND = 138, -- IDB_RES_BACKGROUND
}

-- apps/xOptCon/build_TaijiMPC6.py writes the release version here at build time and restores it afterwards.
local _VERSION = "5.2.41.3"
local _dirCompany = "C:\\TaijiControl"
local _dirExeHomeDir = _dirCompany .. "\\TaiJiMPC6"
local _dirExeFullPath = _dirCompany .. "\\TaiJiMPC6\\TaiJiMPC.exe"

local _bCustomPath = false
local _installTaiJiDataSvc = false

local _strResourceCN = {
	SETUP = "泰极MPC安装程序",
	CHOOSE_INSTALL_PATH = "选择安装路径",
	LICENSE = "安装许可协议",
	SPACE_NOT_ENOUGH = "系统空间不足",
	SPACE_HINT = "系统空间不足，需要500M以上空间，请另外选择安装位置",
	MKDIR_FAILED = "创建目录失败: ",
	LOADING = "正在加载",
	HOSTVM_CONFIG_FAILED = "无法生成 HostVM 配置文件: ",
}

local _strResourceEN = {
	SETUP = "Setup",
	CHOOSE_INSTALL_PATH = "Choose Install Path",
	LICENSE = "License Agreement",
	SPACE_NOT_ENOUGH = "System space is not enough",
	SPACE_HINT = "System space is not enough, need more than 500M, please choose another install path",
	MKDIR_FAILED = "Make dir failed: ",
	LOADING = "Loading",
	HOSTVM_CONFIG_FAILED = "Failed to create the HostVM config: ",
}

local _strResource = _strResourceCN

function ResetInstallPath(installPath)
    _dirCompany = installPath
    _dirExeHomeDir = _dirCompany .. "\\TaiJiMPC6"
    _dirExeFullPath = _dirCompany .. "\\TaiJiMPC6\\TaiJiMPC.exe"
end

function OnInitialize()
    _dirCompany = installx.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl", "InstallPath")
    if (_dirCompany == nil or _dirCompany == "") then
        _dirCompany = "C:\\TaijiControl"
    end
    ResetInstallPath(_dirCompany)
    installx.DuiText("pathedit", _dirCompany)
	local acp = installx.SysACP()
	if acp == 936 then
		_strResource = _strResourceCN
	else
		_strResource = _strResourceEN
	end
	installx.LogPrint("ACP: ", acp)
end

function OnButtonClick(btnName)
	installx.LogPrint("OnButtonClick: ", btnName)
    if (btnName == "custombtn") then
        _bCustomPath = not _bCustomPath
        installx.DuiVisible("customlayout", _bCustomPath)
        if (_bCustomPath) then
            installx.DuiWindowPos("_MainFrame", 0, 0, 600, 428)
        else
            installx.DuiWindowPos("_MainFrame", 0, 0, 600, 380)
        end
    elseif (btnName == "browfilebtn") then
        installPath = installx.FilePathChoose(_strResource.CHOOSE_INSTALL_PATH, _dirCompany)
        installx.DuiText("pathedit", installPath)
        ResetInstallPath(installPath)
    elseif (btnName == "closebtn") then
        if (installx.DuiTabSelect("installlayout") > 0) then
            installx.DuiTabSelect("installlayout", 0)
            installx.DuiVisible("custombtn", true)
            installx.DuiText("titletext", _strResource.SETUP)
            installx.DuiTextColor("titletext", 0xFFFFFFFF)
            installx.DuiSetBkImage("mainlayout", "res='" .. RES.BACKGROUND .. "' restype='png' source='0,0,600,221' corner='300,208,5,5'")
        else
            installx.DuiMessage(0x10, 0, 0) -- WM_CLOSE
        end
    elseif (btnName == "lookprotbtn") then
        if (installx.DuiVisible("customlayout")) then
            CustomLayeroutHide()
        end
        installx.DuiTabSelect("installlayout", 1)
        installx.DuiVisible("custombtn", false)
        installx.DuiText("titletext", _strResource.LICENSE)
        installx.DuiTextColor("titletext", 0xFF5DA5FF)
        installx.DuiText("portcontent", RES.LICENSE)
        installx.DuiSetBkImage("mainlayout", "res='" .. RES.BACKGROUND .. "' restype='png' source='20,210,80,216'")
    elseif (btnName == "starinstallbtn") then
        StartSetup()
    elseif (btnName == "starusebtn") then
        _FinishInstall()
	elseif (btnName == "bootstartbtn") then
	elseif (btnName == "runbtn") then
	elseif (btnName == "showtxbtn") then
    end
end

function OnSelChanged(btnName, isSelected)
    if (btnName == "protcheckbtn") then
        installx.DuiEnable("starinstallbtn", isSelected)
    end
end

local _image_index = 0
local _lastPercent = 0   -- prevents progress bar going backwards during parallel extraction

-- Progress ranges based on actual 7z file sizes (Win32:163MB, WinPy313:1121MB, TaiJiDataSvc:29MB, total ~1312MB)
-- SOFT1 (Win32.7z,  163MB): 5%~16%  (11 points, ~12%)
-- SOFT2 (WinPy313.7z, 1121MB): 16%~90% (74 points, ~85%)
-- SOFT3 (TaiJiDataSvc.7z, 29MB): 90%~92% (2 points, ~3%)

function OnUnzipProgress(nNotifyID, nTotalFileNum, nCurFileIndex, nTotalSize, nCurrentSize)
	-- If this is the callback with completion values (-1), skip updating progress here
	if nCurFileIndex == -1 or nTotalFileNum == -1 then
		return
	end

	local percent

	if nNotifyID == RES.SOFT1 then
		percent = 5 + math.floor(11 * nCurFileIndex / nTotalFileNum)
	elseif nNotifyID == RES.SOFT2 then
		percent = 16 + math.floor(74 * nCurFileIndex / nTotalFileNum)
	elseif nNotifyID == RES.SOFT3 then
		percent = 90 + math.floor(2 * nCurFileIndex / nTotalFileNum)
	else
		return
	end
	-- Clamp to avoid backwards steps when packages extract in parallel
	if percent <= _lastPercent then return end
	_lastPercent = percent

	local bgRes = "res='" .. RES.BACKGROUND .. "' restype='png'"
	if _image_index < 3 and percent > 80 then
		_image_index = 3
		installx.DuiSetBkImage("mainlayout", bgRes .. " source='0,666,600,886' corner='300,208,5,5'")
	elseif _image_index < 2 and percent > 50 then
		_image_index = 2
		installx.DuiSetBkImage("mainlayout", bgRes .. " source='0,444,600,664' corner='300,208,5,5'")
	elseif _image_index < 1 and percent > 15 then
		_image_index = 1
		installx.DuiSetBkImage("mainlayout", bgRes .. " source='0,222,600,442' corner='300,208,5,5'")
	end
	installx.DuiProgress("installprogress", percent, _strResource.LOADING  .. " " .. percent .. "%" )
end

function CheckDiskSpace()
    freeSystemSpace = installx.DiskFreeSpace("")
    if (freeSystemSpace <= 10*1024*1024) then
        ErrorHint(_strResource.SPACE_NOT_ENOUGH)
        return false
    end
    freeSpace = installx.DiskFreeSpace(_dirCompany) or 500 * 1024 * 1024
    if (freeSpace < 500 * 1024 * 1024) then
        ErrorHint(_strResource.SPACE_HINT)
        return false
    end
    return true
end

function CustomLayeroutHide()
    installx.DuiVisible("customlayout", false)
    installx.DuiWindowPos("_MainFrame", 0, 0, 600, 380)
end

function KillProcesses()

	installx.LogPrint("KillProcesses TaiJiOPCSim.exe ...")
    installx.ProcessKill("TaiJiOPCSim.exe")

	installx.LogPrint("KillProcesses TaiJiMPC.exe ...")
    installx.ProcessKill("TaiJiMPC.exe")

	installx.ProcessExecute("\"" .. _dirCompany .. "\\HostVM\\HostVM.exe\" --stop HostVM")
	installx.ProcessExecute("\"" .. _dirCompany .. "\\HostVM\\HostVM.exe\" service stop HostVM")
	installx.ProcessExecute("\"" .. _dirCompany .. "\\HostVM\\HostVM.exe\" --uninstall HostVM")
	installx.ProcessExecute("\"" .. _dirCompany .. "\\HostVM\\HostVM.exe\" service remove HostVM")

	installx.LogPrint("KillProcesses HostVM.exe ...")
    installx.ProcessKill("HostVM.exe")

end

function StartSetup()

	installx.LogPrint("Require Admin Promote ...")
    isAdmin = installx.RunAsAdmin()

    KillProcesses()

	installx.LogPrint("CheckDiskSpace ...")
    if ( not CheckDiskSpace() ) then 
        return
    end

	installx.LogPrint("Check Exists " .. _dirCompany .. " ...")
	if not installx.FilePathExists(_dirCompany) then
		installx.LogPrint("Makedir " .. _dirCompany .. " ...")
		if (not installx.FilePathMkdir(_dirCompany)) then
			ErrorHint(_strResource.MKDIR_FAILED .. _dirCompany)
			return
		end
	end

    CustomLayeroutHide()
    installx.DuiVisible("custombtn", false)
    installx.DuiVisible("errortiplab", false)

    installx.DuiEnable("closebtn", false)
    installx.DuiVisible("starinstallbtn", false)
    installx.DuiVisible("sureportlayout", false)
    installx.DuiVisible("installprogress", true)

	local _dirTaiJiDataSvc = _dirCompany .. "\\TaiJiDataSvc"
	_installTaiJiDataSvc = not installx.FilePathExists(_dirTaiJiDataSvc)

	local resourceIDs = {RES.SOFT1, RES.SOFT2}
	local skipPrefixes = nil
	if _installTaiJiDataSvc then
		installx.LogPrint("Include Resource ID " .. RES.SOFT3 .. " (TaiJiDataSvc.7z)...")
		table.insert(resourceIDs, RES.SOFT3)
	else
		skipPrefixes = {"TaiJiDataSvc"}
		installx.LogPrint("Skip TaiJiDataSvc.7z, directory already exists: " .. _dirTaiJiDataSvc)
		installx.LogPrint("Skip TaiJiDataSvc entries from bundled archives as well")
	end

	installx.LogPrint("Unzip selected resources in one pass...")
	installx.FilePathUnzip(resourceIDs, _dirCompany, skipPrefixes)
end

-- The package ships HostVM\hostvm.default.xml but never hostvm.xml: once hostvm.xml exists it belongs
-- to the site (ports, thread counts, the ident secret key), so an upgrade must keep it. Only a fresh
-- install, where hostvm.xml does not exist yet, starts from the bundled default.
-- Returns false when a fresh install cannot get a hostvm.xml, after showing the error.
function InstallHostVMConfig()
    local defaultConfig = _dirCompany .. "\\HostVM\\hostvm.default.xml"
    local siteConfig = _dirCompany .. "\\HostVM\\hostvm.xml"
    if installx.FilePathExists(siteConfig) then
        installx.LogPrint("Keep existing HostVM config: " .. siteConfig)
        return true
    end
    -- FilePathCopy passes its third argument to CopyFile as bFailIfExists, so it is not relied on
    -- here; the existence check above is what keeps a site config from being overwritten.
    if installx.FilePathCopy(defaultConfig, siteConfig) then
        installx.LogPrint("Created HostVM config from " .. defaultConfig)
        return true
    end
    ErrorHint(_strResource.HOSTVM_CONFIG_FAILED .. siteConfig)
    return false
end

-- OPC core components come in two Graybox packages, one per bitness, and each carries what the
-- other lacks:
--   x86: OpcEnum.exe (the OPC server browser, which only exists as 32-bit) and the 32-bit
--        proxy/stubs. Needed by the 32-bit DA servers TaiJiOPCSim and TaiJiPYSim\opcserver_win_i386,
--        and by every client that browses servers through OpcEnum.
--   x64: the 64-bit proxy/stubs only. Needed by HostVM's DA client (libidh, x64) to talk to DA
--        servers and to the 32-bit OpcEnum.
-- Each package is checked on its own, so a machine where other OPC software already installed the
-- 32-bit half still gets the 64-bit half.
local OPC_IID_IOPCSERVER = "{39C13A4D-011E-11D0-9675-0020AFD8ADB3}"       -- proxy/stub in opcproxy.dll
local OPC_IID_IOPCSERVERLIST = "{13486D50-4821-11D2-A494-3CB306C10000}"   -- proxy/stub in opccomn_ps.dll
local OPC_CLSID_OPCSERVERLIST = "{13486D51-4821-11D2-A494-3CB306C10000}"  -- OpcEnum.exe

-- Default value of an HKCR key, or nil. Not RegGetValue(root, path, ""): that only reports that the
-- key exists, which an empty leftover key passes too.
local function RegDefault(path)
    local value = installx.RegGetDefaultValue(HRootKey.HKEY_CLASSES_ROOT, path)
    if value == nil or value == "" then
        return nil
    end
    return tostring(value)
end

-- True when the file a COM server registration points at exists. LocalServer32 may be quoted and
-- carry arguments. In the 32-bit view ("WOW6432Node\\") System32 means SysWOW64, which is where a
-- 32-bit process gets redirected; this installer is 64-bit and is not.
local function ComServerFileExists(view, serverPath)
    if serverPath == nil then
        return false
    end
    local path = serverPath:match('^"([^"]+)"') or serverPath
    if view ~= "" then
        local first, last = path:lower():find("\\system32\\", 1, true)
        if first then
            path = path:sub(1, first) .. "SysWOW64" .. path:sub(last)
        end
    end
    return installx.FilePathExists(path)
end

-- True when COM in the given registry view ("" or "WOW6432Node\\") can marshal the interface: it
-- names a proxy/stub class, and that class's DLL is on disk.
local function OpcProxyUsable(view, iid)
    local proxyClsid = RegDefault(view .. "Interface\\" .. iid .. "\\ProxyStubClsid32")
    if proxyClsid == nil then
        return false
    end
    return ComServerFileExists(view, RegDefault(view .. "CLSID\\" .. proxyClsid .. "\\InprocServer32"))
end

local function OpcProxiesUsable(view)
    return OpcProxyUsable(view, OPC_IID_IOPCSERVER) and OpcProxyUsable(view, OPC_IID_IOPCSERVERLIST)
end

-- OpcEnum is only ever registered in the 32-bit view.
local function OpcEnumUsable()
    local view = "WOW6432Node\\"
    return ComServerFileExists(view, RegDefault(view .. "CLSID\\" .. OPC_CLSID_OPCSERVERLIST .. "\\LocalServer32"))
end

function InstallOpcCoreComponents()
    local opcDir = _dirCompany .. "\\Common\\opc\\"
    if OpcProxiesUsable("WOW6432Node\\") and OpcEnumUsable() then
        installx.LogPrint("Skip OPC core components x86, already registered")
    else
        installx.ProcessExecute("\"" .. opcDir .. "GBDA_Install_Prereq_x86.msi\" /quiet")
    end
    if OpcProxiesUsable("") then
        installx.LogPrint("Skip OPC core components x64, already registered")
    else
        installx.ProcessExecute("\"" .. opcDir .. "GBDA_Install_Prereq_x64.msi\" /quiet")
    end
end

-- Returning false tells the installer that setup did not complete: it shows the failure and
-- enables the close button instead of moving on to the finish page.
function PostSetup()

    -- Without hostvm.xml the HostVM service cannot start. Checking it first means a failure stops
    -- before any runtime, service, registry or shortcut is touched.
    if not InstallHostVMConfig() then
        return false
    end

	local percent = 94
	installx.DuiProgress("installprogress", percent, _strResource.LOADING  .. " " .. percent .. "%" )
    local vcRedist = installx.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64", "Installed")
    if (vcRedist == nil or vcRedist == 0) then
        installx.ProcessExecute("\"" .. _dirCompany .. "\\Common\\vcredist\\vc_redist.x64.exe\" /install /quiet /norestart", true, 60)
    end

	local percent = 95
	installx.DuiProgress("installprogress", percent, _strResource.LOADING  .. " " .. percent .. "%" )
    InstallOpcCoreComponents()

	local percent = 96
	installx.DuiProgress("installprogress", percent, _strResource.LOADING  .. " " .. percent .. "%" )

    installx.ProcessExecute("\"" .. _dirCompany .. "\\TaiJiOPCSim\\bin\\TaiJiOPCSim.exe\" -RegServer", false, 10)

	installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes\\WOW6432Node\\CLSID\\{A48A6241-A024-4f99-B105-5DF8CCEA66BA}\\Implemented Categories", "", "")
	installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes\\WOW6432Node\\CLSID\\{A48A6241-A024-4f99-B105-5DF8CCEA66BA}\\Implemented Categories\\{63D5F430-CFE4-11d1-B2C8-0060083BA1FB}", "", "")
	installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes\\WOW6432Node\\CLSID\\{A48A6241-A024-4f99-B105-5DF8CCEA66BA}\\Implemented Categories\\{63D5F432-CFE4-11d1-B2C8-0060083BA1FB}", "", "")

    local haspVersion = installx.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Aladdin Knowledge Systems\\HASP\\Driver\\Installer", "DrvPkgVersion")
    if (haspVersion == nil or haspVersion < "8.31") then --os.execute
        installx.ProcessExecute("\"" .. _dirCompany .. "\\Common\\hasp\\haspdinst.exe\" -install -nomsg")
    end

	RunBatchFile(_dirCompany .. "\\HostVM\\install_python_env.bat", 10)
	if _installTaiJiDataSvc then
		RunBatchFile(_dirCompany .. "\\TaiJiDataSvc\\install_TaiJiDataSvc.bat", 30)
	else
		installx.LogPrint("Skip install_TaiJiDataSvc.bat, TaiJiDataSvc already exists")
	end
	RunBatchFile(_dirCompany .. "\\TaiJiDataClient\\install_TaiJiDataClient.bat", 30)
	RunBatchFile(_dirCompany .. "\\TaijiPYSim\\install_taijiopcsim.bat", 30)

	local percent = 97
	installx.DuiProgress("installprogress", percent, _strResource.LOADING  .. " " .. percent .. "%" )

    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl", "InstallPath", _dirCompany)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl\\TaiJiMPC6", "APPPath", _dirExeFullPath)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl\\TaiJiMPC6", "Version", _VERSION)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl\\PythonEnv", "InstallPath", _dirCompany .. "\\WinPy313\\python")
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl\\PythonEnv", "Version", "3.13")

	installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\WOW6432Node\\TaijiControl", "InstallPath", _dirCompany)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\WOW6432Node\\TaijiControl\\TaiJiMPC6", "APPPath", _dirExeFullPath)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\WOW6432Node\\TaijiControl\\TaiJiMPC6", "Version", _VERSION)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\WOW6432Node\\TaijiControl\\PythonEnv", "InstallPath", _dirCompany .. "\\WinPy313\\python")
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\WOW6432Node\\TaijiControl\\PythonEnv", "Version", "3.13")

    local desktopDir = installx.FilePathGetSpecialLocation(CSIDL.COMMON_DESKTOPDIRECTORY)
    installx.FilePathCreateShortCut(desktopDir .. "\\TaiJiMPC6.lnk", _dirExeFullPath, _dirExeHomeDir, "TaiJiMPC6")

    local startMenuDir = installx.FilePathGetSpecialLocation(CSIDL.COMMON_STARTMENU)
    installx.FilePathMkdir(startMenuDir .. "\\Programs\\TaijiControl")
    installx.FilePathCreateShortCut(startMenuDir .. "\\Programs\\TaijiControl\\TaiJiMPC6.lnk", _dirExeFullPath, _dirExeHomeDir, "TaiJiMPC6")

	local percent = 98
	installx.DuiProgress("installprogress", percent, _strResource.LOADING  .. " " .. percent .. "%" )
    installx.ProcessExecute("\"" .. _dirCompany .. "\\HostVM\\HostVM.exe\" service install HostVM")
    installx.ProcessExecute("\"" .. _dirCompany .. "\\HostVM\\HostVM.exe\" service start HostVM")

	local percent = 99
	installx.DuiProgress("installprogress", percent, _strResource.LOADING  .. " " .. percent .. "%" )

    -- The first RegSetValue below creates the TaiJiMPC6 key.
    local UNINST_KEY = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\TaiJiMPC6"
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayIcon", _dirExeFullPath)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayName", "Tai-Ji MPC6")
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayVersion", _VERSION)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "Publisher", "Tai-Ji Soft")
    -- build_TaijiMPC6.py puts UnInstall.exe into TaijiMPC6\ of Win32.7z.
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "UninstallString", _dirExeHomeDir .. "\\UnInstall.exe")

	local percent = 100
	installx.DuiProgress("installprogress", percent, _strResource.LOADING  .. " " .. percent .. "%" )

end

function _FinishInstall()
    local runSelected = installx.DuiOptionSelect("runbtn")
    if (runSelected) then
        installx.ProcessExecute(_dirExeFullPath, false, 0)
    end

    local runOnStartSelected = installx.DuiOptionSelect("bootstartbtn")
    if (runOnStartSelected) then
        installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "TaijiMPC", _dirExeFullPath)
    else
        installx.RegDeleteValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "TaijiMPC")
    end

    local setPyEnv = installx.DuiOptionSelect("pyenvbtn")
    if (setPyEnv) then
        local issucess, errdesc = installx.SysPathAdd(_dirCompany .. "\\WinPy313\\python")
        if not issucess then
            installx.LogPrint(errdesc)
        end

        issucess, errdesc = installx.SysPathAdd(_dirCompany .. "\\WinPy313\\python\\Scripts")
        if not issucess then
            installx.LogPrint(errdesc)
        end

        issucess, errdesc = installx.SysEnvSet("PYTHONHOME", _dirCompany .. "\\WinPy313\\python")
        if not issucess then
            installx.LogPrint(errdesc)
        end
    end

    showTxtSelected = installx.DuiOptionSelect("showtxbtn")
    if showTxtSelected then
        -- show update ？ show install log?
    end
end

function QueryByKey(keyName)
    if (keyName == "InstallPath") then
        return _dirCompany
    end
end
