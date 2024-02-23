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

dirCompany = "C:\\TaijiControl"
dirExeHomeDir = dirCompany .. "\\TaiJiMPC5"
dirExeFullPath = dirCompany .. "\\TaiJiMPC5\\TaiJiMPC.exe"
dirPreCompany = dirCompany

_bCustomPath = false

function ResetInstallPath(installPath)
    dirCompany = installPath
    dirExeHomeDir = dirCompany .. "\\TaiJiMPC5"
    dirExeFullPath = dirCompany .. "\\TaiJiMPC5\\TaiJiMPC.exe"
end

function OnInitialize()
    dirCompany = install.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl", "InstallPath")
    if (dirCompany == nil or dirCompany == "") then
        dirCompany = "C:\\TaijiControl"
    end
    dirPreCompany = dirCompany
    ResetInstallPath(dirCompany)
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
        --freeSystemSpace = install.DiskFreeSpace("")
        --freeSpace = install.DiskFreeSpace(installPath)
        --install.LogPrint(freeSystemSpace, freeSpace)
    elseif (btnName == "closebtn") then
        install.DuiText("titletext", "安装程序")
        install.DuiTextColor("titletext", 0xFFFFFFFF)
        install.DuiSetBkImage("mainlayout", "res='130' restype='png' source='0,0,600,220' corner='300,208,300,5'")
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
    elseif (btnName == "starusebtn") then
        _FinishInstall()
    end
end

function OnSelChanged(btnName, isSelected)
    if (btnName == "protcheckbtn") then
        install.DuiEnable("starinstallbtn", isSelected)
    end
end


function PreSetup()
    install.ProcessKill("TaiJiMPC.exe")
    install.ProcessKill("TaiJiOPCSim.exe")
    install.ProcessExecute(dirCompany .. "\\HostVM\\HostVM.exe --stop HostVM")
    install.ProcessKill("HostVM.exe")
end

function PostSetup()

    opcEnum = install.RegGetValue(HRootKey.HKEY_CLASSES_ROOT, "CLSID\\{13486D50-4821-11D2-A494-3CB306C10000}", "")
        or install.RegGetValue(HRootKey.HKEY_CLASSES_ROOT, "WOW6432Node\\CLSID\\{13486D50-4821-11D2-A494-3CB306C10000}", "")
    if (opcEnum == nil or opcEnum == False) then
        install.ProcessExecute(dirCompany .. "\\Common\\opc\\GBDA_Install_Prereq_x86.msi /quiet")
    end

    install.ProcessExecute(dirCompany .. "\\TaiJiOPCSim\\bin\\TaiJiOPCSim.exe -RegServer")

    haspVersion = install.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Aladdin Knowledge Systems\\HASP\\Driver\\Installer", "DrvPkgVersion")
    if (haspVersion == nil or haspVersion < "8.31") then
        --os.execute
        install.ProcessExecute(dirCompany .. "\\Common\\hasp\\haspdinst.exe -install -nomsg")
    end

    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl", "InstallPath", dirCompany)
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl\\TaiJiMPC5", "APPPath", dirExeFullPath)
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl\\TaiJiMPC5", "Version", install.GetVersion())

    desktopDir = install.GetSpecialFolderLocation(CSIDL_Enum.CSIDL_COMMON_DESKTOPDIRECTORY)
    install.CreateShortCut(desktopDir .. "\\TaiJiMPC5.lnk", dirExeFullPath, dirExeHomeDir, "TaiJiMPC5")

    startMenuDir = install.GetSpecialFolderLocation(CSIDL_Enum.CSIDL_COMMON_STARTMENU)
    install.CreateDirectory(startMenuDir .. "\\Programs\\TaijiControl")
    install.CreateShortCut(startMenuDir .. "\\Programs\\TaijiControl\\TaiJiMPC5.lnk", dirExeFullPath, dirExeHomeDir, "TaiJiMPC5")

    install.ProcessExecute(dirCompany .. "\\HostVM\\HostVM.exe --install HostVM")
    install.ProcessExecute(dirCompany .. "\\HostVM\\HostVM.exe --start HostVM")

    UNINST_KEY = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "TaiJiMPC5")
    UNINST_KEY = UNINST_KEY .. "\\TaiJiMPC5"
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayIcon", dirExeFullPath)
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayName", "TaijiMPC5")
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "DisplayVersion", install.GetVersion())
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "Publisher", "TaijiSoft")
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, UNINST_KEY, "UninstallString", dirCompany .. "\\TaiJiMPC5\\UnInstall.exe")
end

function _FinishInstall()
    runSelected = install.DuiOptionSelect("runbtn")
    if (runSelected) then
        install.ProcessExecute(dirExeFullPath, false, 0)
    end

    runOnStartSelected = install.DuiOptionSelect("bootstartbtn")
    if (runOnStartSelected) then
        install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "TaijiMPC", dirExeFullPath)
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
        return dirCompany
    end
end
