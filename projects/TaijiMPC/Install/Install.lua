local HRootKey = {
    HKEY_CLASSES_ROOT = 0,
    HKEY_CURRENT_USER = 1,
    HKEY_LOCAL_MACHINE = 2,
    HKEY_USERS = 3,
    HKEY_CURRENT_CONFIG = 4
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

_dirCompany = "C:\\TaijiControl"
_dirExeHomeDir = _dirCompany .. "\\TaiJiMPC5"
_dirExeFullPath = _dirCompany .. "\\TaiJiMPC5\\TaiJiMPC.exe"
_dirPreCompany = _dirCompany

_bCustomPath = false

function ResetInstallPath(installPath)
    _dirCompany = installPath
    _dirExeHomeDir = _dirCompany .. "\\TaiJiMPC5"
    _dirExeFullPath = _dirCompany .. "\\TaiJiMPC5\\TaiJiMPC.exe"
end

function OnInitialize()
    _dirCompany = install.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl", "InstallPath")
    if (_dirCompany == nil or _dirCompany == "") then
        _dirCompany = "C:\\TaijiControl"
    end
    _dirPreCompany = _dirCompany
    ResetInstallPath(_dirCompany)
end

function OnButtonClick(btnName)
    if (btnName == "custombtn") then
        _bCustomPath = not _bCustomPath
        install.DuiVisible("customlayout", _bCustomPath)
        if (_bCustomPath) then
            install.DuiWindowPos("_MainFrame", 0, 0, 600, 428)
        else
            install.DuiWindowPos("_MainFrame", 0, 0, 600, 380)
        end
    elseif (btnName == "browfilebtn") then
        installPath = install.FilePathChoose("选择安装目录", "C:\\TaijiControl")
        install.DuiText("pathedit", installPath)
        ResetInstallPath(installPath)
    elseif (btnName == "closebtn") then
        if (install.DuiTabSelect("installlayout") > 0) then
            install.DuiTabSelect("installlayout", 0)
            install.DuiVisible("custombtn", true)
            install.DuiText("titletext", "安装程序")
            install.DuiTextColor("titletext", 0xFFFFFFFF)
            install.DuiSetBkImage("mainlayout", "res='130' restype='png' source='0,0,600,220' corner='300,208,300,5'")
        else
            
        end
    elseif (btnName == "lookprotbtn") then
        if (install.DuiVisible("customlayout")) then
            install.DuiVisible("customlayout", false)
            install.DuiWindowPos("_MainFrame", 0, 0, 600, 380)
        end
        install.DuiTabSelect("installlayout", 1)
        install.DuiVisible("custombtn", false)
        install.DuiText("titletext", "安装许可协议")
        install.DuiTextColor("titletext", 0xFF3B3B3B)
        install.DuiText("portcontent", 133) -- IDR_REGCONTENT1
        install.DuiSetBkImage("mainlayout", "res='130' restype='png' source='20,210,80,216'")
    elseif (btnName == "starinstallbtn") then
        isAdmin = install.RunAsAdmin()
        PrepareSetup()
        if ( not CheckDiskSpace() ) then 
            return
        end
        if (not install.FilePathMkdir(_dirCompany)) then
            ErrorHint("创建目录失败: " .. _dirCompany)
            return
        end
    elseif (btnName == "starusebtn") then
        _FinishInstall()
    end
end

function OnSelChanged(btnName, isSelected)
    if (btnName == "protcheckbtn") then
        install.DuiEnable("starinstallbtn", isSelected)
    end
end

function ErrorHint(txtError)
    install.DuiText("errortiplab", txtError)
end

function CheckDiskSpace()
    freeSystemSpace = install.DiskFreeSpace("")
    if (freeSystemSpace <= 10*1024*1024) then
        ErrorHint("系统空间不足")
        return false
    end
    freeSpace = install.DiskFreeSpace(_dirCompany)
    if (freeSpace < 500 * 1024 * 1024) then
        ErrorHint("磁盘空间不足，需要500M以上空间，请另外选择安装位置")
        return false
    end
    return true
end

function PrepareSetup()
    install.ProcessKill("TaiJiMPC.exe")
    install.ProcessKill("TaiJiOPCSim.exe")
    install.ProcessExecute(_dirCompany .. "\\HostVM\\HostVM.exe --stop HostVM")
    install.ProcessKill("HostVM.exe")
end

function PostSetup()

    opcEnum = install.RegGetValue(HRootKey.HKEY_CLASSES_ROOT, "CLSID\\{13486D50-4821-11D2-A494-3CB306C10000}", "")
        or install.RegGetValue(HRootKey.HKEY_CLASSES_ROOT, "WOW6432Node\\CLSID\\{13486D50-4821-11D2-A494-3CB306C10000}", "")
    if (opcEnum == nil or opcEnum == False) then
        install.ProcessExecute(_dirCompany .. "\\Common\\opc\\GBDA_Install_Prereq_x86.msi /quiet")
    end

    install.ProcessExecute(_dirCompany .. "\\TaiJiOPCSim\\bin\\TaiJiOPCSim.exe -RegServer")

    haspVersion = install.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Aladdin Knowledge Systems\\HASP\\Driver\\Installer", "DrvPkgVersion")
    if (haspVersion == nil or haspVersion < "8.31") then
        --os.execute
        install.ProcessExecute(_dirCompany .. "\\Common\\hasp\\haspdinst.exe -install -nomsg")
    end

    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl", "InstallPath", _dirCompany)
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl\\TaiJiMPC5", "APPPath", _dirExeFullPath)
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl\\TaiJiMPC5", "Version", install.GetVersion())

    desktopDir = install.GetSpecialFolderLocation(CSIDL_Enum.CSIDL_COMMON_DESKTOPDIRECTORY)
    install.CreateShortCut(desktopDir .. "\\TaiJiMPC5.lnk", _dirExeFullPath, _dirExeHomeDir, "TaiJiMPC5")

    startMenuDir = install.GetSpecialFolderLocation(CSIDL_Enum.CSIDL_COMMON_STARTMENU)
    install.CreateDirectory(startMenuDir .. "\\Programs\\TaijiControl")
    install.CreateShortCut(startMenuDir .. "\\Programs\\TaijiControl\\TaiJiMPC5.lnk", _dirExeFullPath, _dirExeHomeDir, "TaiJiMPC5")

    install.ProcessExecute(_dirCompany .. "\\HostVM\\HostVM.exe --install HostVM")
    install.ProcessExecute(_dirCompany .. "\\HostVM\\HostVM.exe --start HostVM")

    UNINST_KEY = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "TaiJiMPC5")
    UNINST_KEY = UNINST_KEY .. "\\TaiJiMPC5"
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayIcon", _dirExeFullPath)
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayName", "TaijiMPC5")
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayVersion", install.GetVersion())
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "Publisher", "TaijiSoft")
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "UninstallString", _dirCompany .. "\\TaiJiMPC5\\UnInstall.exe")
end

function _FinishInstall()
    runSelected = install.DuiOptionSelect("runbtn")
    if (runSelected) then
        install.ProcessExecute(_dirExeFullPath, false, 0)
    end

    runOnStartSelected = install.DuiOptionSelect("bootstartbtn")
    if (runOnStartSelected) then
        install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "TaijiMPC", _dirExeFullPath)
    else
        install.RegDeleteValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "TaijiMPC")
    end

    showTxtSelected = install.DuiOptionSelect("showtxbtn")
    if showTxtSelected then
        -- show update ？ show install log?
    end
end



function QueryByKey(keyName)
    if (keyName == "InstallPath") then
        return _dirCompany
    end
end
