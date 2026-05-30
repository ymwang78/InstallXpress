-- HRootKey, CSIDL, RunBatchFile, ErrorHint are provided by the framework base library.

-- Resource ID constants (mirrors resource.h)
local RES = {
    LICENSE  = 133,  -- IDR_REGCONTENT1
    SOFT1    = 134,  -- IDR_INSTALLSOFT1 (Rto.7z)
    SOFT2    = 139,  -- IDR_INSTALLSOFT2 (WinPy312.7z)
    SOFT3    = 140,  -- IDR_INSTALLSOFT3 (data.7z)
    BACKGROUND = 138, -- IDB_RES_BACKGROUND
}

local _VERSION = "1.0.0.0"
local _dirCompany = "C:\\ZJU"
local _dirExeHomeDir = _dirCompany .. "\\Rto"
local _dirExeFullPath = _dirCompany .. "\\Rto\\xRto.exe"

local _bCustomPath = false
local _installData = false
local _activeResources = {}
local _resourceProgress = {}
local _lastLoggedPercent = -1

local _strResourceCN = {
	SETUP = "浙大RTO安装程序",
	CHOOSE_INSTALL_PATH = "选择安装路径",
	LICENSE = "安装许可协议",
	SPACE_NOT_ENOUGH = "系统空间不足",
	SPACE_HINT = "系统空间不足，需要5G以上空间，请另外选择安装位置",
	MKDIR_FAILED = "创建目录失败: ",
	LOADING = "正在加载",
}

local _strResourceEN = {
	SETUP = "Setup",
	CHOOSE_INSTALL_PATH = "Choose Install Path",
	LICENSE = "License Agreement",
	SPACE_NOT_ENOUGH = "System space is not enough",
	SPACE_HINT = "System space is not enough, need more than 500M, please choose another install path",
	MKDIR_FAILED = "Make dir failed: ",
	LOADING = "Loading",
}

local _strResource = _strResourceCN

function ResetInstallPath(installPath)
    _dirCompany = installPath
    _dirExeHomeDir = _dirCompany .. "\\Rto"
    _dirExeFullPath = _dirCompany .. "\\Rto\\xRto.exe"
end

function OnInitialize()
    _dirCompany = installx.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\ZJU", "InstallPath")
    if (_dirCompany == nil or _dirCompany == "") then
        _dirCompany = "C:\\ZJU"
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

-- Progress weights based on actual 7z file sizes:
-- Rto:163MB, WinPy312:1121MB, Data:29MB. Extraction maps to 5%~92%.
local _resourceWeights = {
	[RES.SOFT1] = 163,
	[RES.SOFT2] = 1121,
	[RES.SOFT3] = 29,
}

function UpdateInstallProgress(percent)
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

	if percent ~= _lastLoggedPercent then
		_lastLoggedPercent = percent
		installx.LogPrint("Progress: " .. percent .. "%")
	end
end

function CalculateExtractionPercent()
	local totalWeight = 0
	local doneWeight = 0
	for _, resourceID in ipairs(_activeResources) do
		local weight = _resourceWeights[resourceID] or 0
		totalWeight = totalWeight + weight
		doneWeight = doneWeight + weight * (_resourceProgress[resourceID] or 0)
	end
	if totalWeight <= 0 then
		return 5
	end
	return 5 + math.floor(87 * doneWeight / totalWeight)
end

function OnUnzipProgress(nNotifyID, nTotalFileNum, nCurFileIndex, nTotalSize, nCurrentSize)
	-- If this is the callback with completion values (-1), skip updating progress here.
	if nCurFileIndex == -1 or nTotalFileNum == -1 then
		return
	end
	if _resourceWeights[nNotifyID] == nil or nTotalFileNum <= 0 then
		return
	end

	local progress = (nCurFileIndex + 1) / nTotalFileNum
	if progress < 0 then progress = 0 end
	if progress > 1 then progress = 1 end
	if progress > (_resourceProgress[nNotifyID] or 0) then
		_resourceProgress[nNotifyID] = progress
	end

	UpdateInstallProgress(CalculateExtractionPercent())
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
	installx.LogPrint("KillProcesses xRto.exe ...")
    installx.ProcessKill("xRto.exe")
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

	local _dirData = _dirCompany .. "\\Rto\\data"
	_installData = not installx.FilePathExists(_dirData)

	local resourceIDs = {RES.SOFT1, RES.SOFT2}
	local skipPrefixes = nil
	if _installData then
		installx.LogPrint("Include Resource ID " .. RES.SOFT3 .. " (data.7z)...")
		table.insert(resourceIDs, RES.SOFT3)
	else
		skipPrefixes = {"data"}
		installx.LogPrint("Skip data.7z, directory already exists: " .. _dirData)
		installx.LogPrint("Skip data entries from bundled archives as well")
	end
	_activeResources = resourceIDs
	_resourceProgress = {}
	for _, resourceID in ipairs(_activeResources) do
		_resourceProgress[resourceID] = 0
	end

	installx.LogPrint("Unzip selected resources in one pass...")
	UpdateInstallProgress(5)
	installx.FilePathUnzip(resourceIDs, _dirCompany, skipPrefixes)
end

function PostSetup()

	UpdateInstallProgress(94)
    local vcRedist = installx.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64", "Installed")
    if (vcRedist == nil or vcRedist == 0) then
        installx.ProcessExecute("\"" .. _dirCompany .. "\\Common\\redist\\vc_redist.x64.exe\" /install /quiet /norestart", true, 60)
    end

	UpdateInstallProgress(95)
    local opcEnum = installx.RegGetValue(HRootKey.HKEY_CLASSES_ROOT, "WOW6432Node\\CLSID\\{13486D50-4821-11D2-A494-3CB306C10000}", "")
    -- local opcEnum = installx.RegGetValue(HRootKey.HKEY_CLASSES_ROOT, "CLSID\\{13486D50-4821-11D2-A494-3CB306C10000}", "")
    if (opcEnum == nil or opcEnum == False) then
        installx.ProcessExecute("\"" .. _dirCompany .. "\\Common\\opc\\GBDA_Install_Prereq_x64.msi\" /quiet")
    end

	UpdateInstallProgress(96)


	UpdateInstallProgress(97)

    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\ZJU", "InstallPath", _dirCompany)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\ZJU\\Rto", "APPPath", _dirExeFullPath)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\ZJU\\Rto", "Version", _VERSION)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\ZJU\\PythonEnv", "InstallPath", _dirCompany .. "\\WinPy312\\python")
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\ZJU\\PythonEnv", "Version", "3.12")

    local desktopDir = installx.FilePathGetSpecialLocation(CSIDL.COMMON_DESKTOPDIRECTORY)
    installx.FilePathCreateShortCut(desktopDir .. "\\Rto.lnk", _dirExeFullPath, _dirExeHomeDir, "Rto")

    local startMenuDir = installx.FilePathGetSpecialLocation(CSIDL.COMMON_STARTMENU)
    installx.FilePathMkdir(startMenuDir .. "\\Programs\\ZJU")
    installx.FilePathCreateShortCut(startMenuDir .. "\\Programs\\ZJU\\Rto.lnk", _dirExeFullPath, _dirExeHomeDir, "Rto")

	UpdateInstallProgress(99)

    local UNINST_KEY = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "Rto")
    UNINST_KEY = UNINST_KEY .. "\\Rto"
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayIcon", _dirExeFullPath)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayName", "RTO")
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayVersion", _VERSION)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "Publisher", "ZJU")
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "UninstallString", _dirCompany .. "\\Rto\\UnInstall.exe")

	UpdateInstallProgress(100)

end

function _FinishInstall()
    local runSelected = installx.DuiOptionSelect("runbtn")
    if (runSelected) then
        installx.ProcessExecute(_dirExeFullPath, false, 0)
    end

    local runOnStartSelected = installx.DuiOptionSelect("bootstartbtn")
    if (runOnStartSelected) then
        installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "Rto", _dirExeFullPath)
    else
        installx.RegDeleteValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "Rto")
    end

    local setPyEnv = installx.DuiOptionSelect("pyenvbtn")
    if (setPyEnv) then
        local issucess, errdesc = installx.SysPathAdd(_dirCompany .. "\\WinPy312\\python")
        if not issucess then
            installx.LogPrint(errdesc)
        end

        issucess, errdesc = installx.SysPathAdd(_dirCompany .. "\\WinPy312\\python\\Scripts")
        if not issucess then
            installx.LogPrint(errdesc)
        end

        issucess, errdesc = installx.SysEnvSet("PYTHONHOME", _dirCompany .. "\\WinPy312\\python")
        if not issucess then
            installx.LogPrint(errdesc)
        end
    end

    showTxtSelected = installx.DuiOptionSelect("showtxbtn")
    if showTxtSelected then
        -- show update or install log?
    end
end

function QueryByKey(keyName)
    if (keyName == "InstallPath") then
        return _dirCompany
    end
end
