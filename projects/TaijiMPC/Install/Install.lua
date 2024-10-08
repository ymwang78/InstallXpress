local HRootKey = {
    HKEY_CLASSES_ROOT = 0,
    HKEY_CURRENT_USER = 1,
    HKEY_LOCAL_MACHINE = 2,
    HKEY_USERS = 3,
    HKEY_CURRENT_CONFIG = 4,
};

local CSIDL_Enum = {
    CSIDL_DESKTOP                   =0,
    CSIDL_INTERNET                  =1,
    CSIDL_PROGRAMS                  =2,
    CSIDL_CONTROLS                  =3,
    CSIDL_PRINTERS                  =4,
    CSIDL_PERSONAL                  =5,
    CSIDL_FAVORITES                 =6,
    CSIDL_STARTUP                   =7,
    CSIDL_RECENT                    =8,
    CSIDL_SENDTO                    =9,
    CSIDL_BITBUCKET                 =10,
    CSIDL_STARTMENU                 =11,
    CSIDL_MYDOCUMENTS               =5,
    CSIDL_MYMUSIC                   =13,
    CSIDL_MYVIDEO                   =14,
    CSIDL_DESKTOPDIRECTORY          =16,
    CSIDL_DRIVES                    =17,
    CSIDL_NETWORK                   =18,
    CSIDL_NETHOOD                   =19,
    CSIDL_FONTS                     =20,
    CSIDL_TEMPLATES                 =21,
    CSIDL_COMMON_STARTMENU          =22,
    CSIDL_COMMON_PROGRAMS           =23,
    CSIDL_COMMON_STARTUP            =24,
    CSIDL_COMMON_DESKTOPDIRECTORY   =25,
    CSIDL_APPDATA                   =26,
    CSIDL_PRINTHOOD                 =27,
    CSIDL_PROGRAM_FILES             =38,
};

local _VERSION = "5.0.39.5"
local _dirCompany = "C:\\TaijiControl"
local _dirExeHomeDir = _dirCompany .. "\\TaiJiMPC5"
local _dirExeFullPath = _dirCompany .. "\\TaiJiMPC5\\TaiJiMPC.exe"

local _bCustomPath = false

local _strResourceCN = {
	SETUP = "泰极MPC安装程序",
	CHOOSE_INSTALL_PATH = "选择安装路径",
	LICENSE = "安装许可协议",
	SPACE_NOT_ENOUGH = "系统空间不足",
	SPACE_HINT = "系统空间不足，需要500M以上空间，请另外选择安装位置",
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
    _dirExeHomeDir = _dirCompany .. "\\TaiJiMPC5"
    _dirExeFullPath = _dirCompany .. "\\TaiJiMPC5\\TaiJiMPC.exe"
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
            installx.DuiSetBkImage("mainlayout", "res='138' restype='png' source='0,0,600,221' corner='300,208,5,5'")
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
        installx.DuiText("portcontent", 133) -- IDR_REGCONTENT1
        installx.DuiSetBkImage("mainlayout", "res='130' restype='png' source='20,210,80,216'")
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
function OnUnzipProgress(nNotifyID, nTotalFileNum, nCurFileIndex, nTotalSize, nCurrentSize)
	local percent = 5 + math.floor(90 * nCurFileIndex / nTotalFileNum)
	if _image_index == 2 and  percent > 80 then
		_image_index = 3
		installx.DuiSetBkImage("mainlayout", "res='138' restype='png' source='0,666,600,886' corner='300,208,5,5'")
	elseif _image_index == 1 and percent > 50 then
		_image_index = 2
		installx.DuiSetBkImage("mainlayout", "res='138' restype='png' source='0,444,600,664' corner='300,208,5,5'")
	elseif _image_index == 0 and percent > 20 then
		_image_index = 1
		installx.DuiSetBkImage("mainlayout", "res='138' restype='png' source='0,222,600,442' corner='300,208,5,5'")
	end
	installx.DuiProgress("installprogress", percent, _strResource.LOADING  .. " " .. percent .. "%" )
end

function ErrorHint(txtError)
	installx.LogPrint(txtError)
    installx.DuiText("errortiplab", txtError)
    installx.DuiVisible("errortiplab", true)
end

function CheckDiskSpace()
    freeSystemSpace = installx.DiskFreeSpace("")
    if (freeSystemSpace <= 10*1024*1024) then
        ErrorHint(_strResource.SPACE_NOT_ENOUGH)
        return false
    end
    freeSpace = installx.DiskFreeSpace(_dirCompany)
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

	installx.LogPrint("KillProcesses TaiJiMPC.exe ...")
    installx.ProcessKill("TaiJiMPC.exe")

	installx.LogPrint("KillProcesses TaiJiOPCSim.exe ...")
    installx.ProcessKill("TaiJiOPCSim.exe")

	installx.LogPrint("Stop Service HostVM ...")
    installx.ProcessExecute("\"" .. _dirCompany .. "\\HostVM\\HostVM.exe\" --stop HostVM")

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

    -- 134: resource id of zip file
	installx.LogPrint("Unzip Resource ID 134...")
    installx.FilePathUnzip(134, _dirCompany, 134)
end

function PostSetup()

	local percent = 95
	installx.DuiProgress("installprogress", percent, _strResource.LOADING  .. " " .. percent .. "%" )

    local opcEnum = installx.RegGetValue(HRootKey.HKEY_CLASSES_ROOT, "CLSID\\{13486D50-4821-11D2-A494-3CB306C10000}", "")
        or installx.RegGetValue(HRootKey.HKEY_CLASSES_ROOT, "WOW6432Node\\CLSID\\{13486D50-4821-11D2-A494-3CB306C10000}", "")
    if (opcEnum == nil or opcEnum == False) then
        installx.ProcessExecute("\"" .. _dirCompany .. "\\Common\\opc\\GBDA_Install_Prereq_x86.msi\" /quiet")
    end

	local percent = 96
	installx.DuiProgress("installprogress", percent, _strResource.LOADING  .. " " .. percent .. "%" )

    installx.ProcessExecute("msiexec /i \"" .. _dirCompany .. "\\Common\\opc\\GBDA_Install_Prereq_x86.msi\" /qn")
    installx.ProcessExecute("\"" .. _dirCompany .. "\\TaiJiOPCSim\\bin\\TaiJiOPCSim.exe\" -RegServer")

    local haspVersion = installx.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Aladdin Knowledge Systems\\HASP\\Driver\\Installer", "DrvPkgVersion")
    if (haspVersion == nil or haspVersion < "8.31") then --os.execute
        installx.ProcessExecute("\"" .. _dirCompany .. "\\Common\\hasp\\haspdinst.exe\" -install -nomsg")
    end

	local percent = 97
	installx.DuiProgress("installprogress", percent, _strResource.LOADING  .. " " .. percent .. "%" )

    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl", "InstallPath", _dirCompany)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl\\TaiJiMPC5", "APPPath", _dirExeFullPath)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl\\TaiJiMPC5", "Version", _VERSION)

    local desktopDir = installx.FilePathGetSpecialLocation(CSIDL_Enum.CSIDL_COMMON_DESKTOPDIRECTORY)
    installx.FilePathCreateShortCut(desktopDir .. "\\TaiJiMPC5.lnk", _dirExeFullPath, _dirExeHomeDir, "TaiJiMPC5")

    local startMenuDir = installx.FilePathGetSpecialLocation(CSIDL_Enum.CSIDL_COMMON_STARTMENU)
    installx.FilePathMkdir(startMenuDir .. "\\Programs\\TaijiControl")
    installx.FilePathCreateShortCut(startMenuDir .. "\\Programs\\TaijiControl\\TaiJiMPC5.lnk", _dirExeFullPath, _dirExeHomeDir, "TaiJiMPC5")

	local percent = 98
	installx.DuiProgress("installprogress", percent, _strResource.LOADING  .. " " .. percent .. "%" )

    installx.ProcessExecute("\"" .. _dirCompany .. "\\HostVM\\HostVM.exe\" --install HostVM")
    installx.ProcessExecute("\"" .. _dirCompany .. "\\HostVM\\HostVM.exe\" --start HostVM")

	local percent = 99
	installx.DuiProgress("installprogress", percent, _strResource.LOADING  .. " " .. percent .. "%" )

    local UNINST_KEY = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "TaiJiMPC5")
    UNINST_KEY = UNINST_KEY .. "\\TaiJiMPC5"
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayIcon", _dirExeFullPath)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayName", "TaijiMPC5")
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayVersion", _VERSION)
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "Publisher", "TaijiSoft")
    installx.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "UninstallString", _dirCompany .. "\\TaiJiMPC5\\UnInstall.exe")

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
        local issucess, errdesc = installx.SysPathAdd(_dirCompany .. "\\Python312")
        if not issucess then
            installx.LogPrint(errdesc)
        end

        issucess, errdesc = installx.SysPathAdd(_dirCompany .. "\\Python312\\Scripts")
        if not issucess then
            installx.LogPrint(errdesc)
        end

        issucess, errdesc = installx.SysEnvSet("PYTHONHOME", _dirCompany .. "\\Python312")
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
