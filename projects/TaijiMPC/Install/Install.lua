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

function ResetInstallPath(installPath)
    dirCompany = installPath
    dirExeHomeDir = dirCompany .. "\\TaiJiMPC5"
    dirExeFullPath = dirCompany .. "\\TaiJiMPC5\\TaiJiMPC.exe"
end

function Initialize()
    dirCompany = install.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl", "InstallPath")
    if (dirCompany == nil or dirCompany == "") then
        dirCompany = "C:\\TaijiControl"
    end
    dirPreCompany = dirCompany
    ResetInstallPath(dirCompany)
end

function PreSetup()
    install.KillProcess("TaiJiMPC.exe")
    install.KillProcess("TaiJiOPCSim.exe")
    install.RunShell(dirCompany .. "\\HostVM\\HostVM.exe --stop HostVM")
    install.KillProcess("HostVM.exe")
end

function PostSetup()

    opcEnum = install.RegGetValue(HRootKey.HKEY_CLASSES_ROOT, "CLSID\\{13486D50-4821-11D2-A494-3CB306C10000}", "")
        or install.RegGetValue(HRootKey.HKEY_CLASSES_ROOT, "WOW6432Node\\CLSID\\{13486D50-4821-11D2-A494-3CB306C10000}", "")
    if (opcEnum == nil or opcEnum == False) then
        install.RunShell(dirCompany .. "\\Common\\opc\\GBDA_Install_Prereq_x86.msi /quiet")
    end

    install.RunShell(dirCompany .. "\\TaiJiOPCSim\\bin\\TaiJiOPCSim.exe -RegServer")

    haspVersion = install.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Aladdin Knowledge Systems\\HASP\\Driver\\Installer", "DrvPkgVersion")
    if (haspVersion == nil or haspVersion < "8.31") then
        --os.execute
        install.RunShell(dirCompany .. "\\Common\\hasp\\haspdinst.exe -install -nomsg")
    end

    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl", "InstallPath", dirCompany)
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl\\TaiJiMPC5", "APPPath", dirExeFullPath)
    install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl\\TaiJiMPC5", "Version", install.GetVersion())

    desktopDir = install.GetSpecialFolderLocation(CSIDL_Enum.CSIDL_COMMON_DESKTOPDIRECTORY)
    install.CreateShortCut(desktopDir .. "\\TaiJiMPC5.lnk", dirExeFullPath, dirExeHomeDir, "TaiJiMPC5")

    startMenuDir = install.GetSpecialFolderLocation(CSIDL_Enum.CSIDL_COMMON_STARTMENU)
    install.CreateDirectory(startMenuDir .. "\\Programs\\TaijiControl")
    install.CreateShortCut(startMenuDir .. "\\Programs\\TaijiControl\\TaiJiMPC5.lnk", dirExeFullPath, dirExeHomeDir, "TaiJiMPC5")

    install.RunShell(dirCompany .. "\\HostVM\\HostVM.exe --install HostVM")
    install.RunShell(dirCompany .. "\\HostVM\\HostVM.exe --start HostVM")

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
    runSelected = install.DuiGetOption("runbtn")
    if (runSelected) then
        install.RunCommand(dirExeFullPath)
    end

    runOnStartSelected = install.DuiGetOption("bootstartbtn")
    if (runOnStartSelected) then
        install.RegSetValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "TaijiMPC", dirExeFullPath)
    else
        install.RegDeleteValue(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "TaijiMPC")
    end

    showTxtSelected = install.DuiGetOption("showtxbtn")
    if showTxtSelected then
        -- show update £¿ show install log?
    end
end

function OnButtonClick(btnName)
    if (btnName == "starusebtn") then
        _FinishInstall()
    end
end

function QueryByKey(keyName)
    if (keyName == "InstallPath") then
        return dirCompany
    end
end
