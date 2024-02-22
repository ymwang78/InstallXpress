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
dirExeHomeDir = dirCompany .. "\\TaiJiMPC"
dirExeFullPath = dirCompany .. "\\TaiJiMPC\\TaiJiMPC.exe"

function Initialize()
    dirCompany = install.RegGetValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl", "InstallPath")
    if (dirCompany == nil or dirCompany == "") then
        dirCompany = "C:\\TaijiControl"
    end
    dirExeHomeDir = dirCompany .. "\\TaiJiMPC"
    dirExeFullPath = dirCompany .. "\\TaiJiMPC\\TaiJiMPC.exe"
end

function ResetInstallPath(installPath)
    dirCompany = installPath
    dirExeHomeDir = dirCompany .. "\\TaiJiMPC"
    dirExeFullPath = dirCompany .. "\\TaiJiMPC\\TaiJiMPC.exe"
end

function PreSetup()
    install.KillProcess("TaiJiMPC.exe")
    install.KillProcess("HostVM.exe")
    install.KillProcess("TaiJiOPCSim.exe")
    install.RunShell(dirCompany .. "\\TaiJiOPCSim\\bin\\TaiJiOPCSim.exe -UnRegServer")
    install.RunShell(dirCompany .. "\\HostVM\\HostVM.exe --uninstall HostVM")
    install.RegDeleteValue(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl", "InstallPath")
    install.RegDeleteKey(HRootKey.HKEY_LOCAL_MACHINE, "Software\\TaijiControl\\TaiJiMPC")
    install.RegDeleteKey(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "TaiJiMPC")
    install.RegDeleteKey(HRootKey.HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\uninstall", "TaiJiMPC")

    desktopDir = install.GetSpecialFolderLocation(CSIDL_Enum.CSIDL_COMMON_DESKTOPDIRECTORY)
    install.DeleteFile(desktopDir .. "\\TaiJiMPC.lnk")

    startMenuDir = install.GetSpecialFolderLocation(CSIDL_Enum.CSIDL_COMMON_STARTMENU)
    install.DeleteDirectory(startMenuDir .. "\\Programs\\TaijiControl")

    --install.DeleteDirectory(dirCompany .. "\\TaiJiMPC")
    install.DeleteDirectory(dirCompany .. "\\TaiJiOPCSim")
    install.DeleteDirectory(dirCompany .. "\\Common")
end

function PostSetup()
end

function OnButtonClick(btnName)
end

function QueryByKey(keyName)
    if (keyName == "InstallPath") then
        return dirCompany
    end
end
