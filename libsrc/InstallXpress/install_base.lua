-- InstallXpress base library
-- Automatically loaded into every installer Lua state before the project script.
-- Defines common constants and utility functions shared across all projects.
--
-- NOTE: The content of this file is also embedded as a C string literal in
-- LuaExtention.cpp (g_install_base_lua). Keep both in sync when editing.
-- TODO: Replace with CMake file(READ ...) to generate the C string automatically.

-- Registry root key indices (matches _hRootKeyID[] in LuaExtention.cpp)
HRootKey = {
    HKEY_CLASSES_ROOT   = 0,
    HKEY_CURRENT_USER   = 1,
    HKEY_LOCAL_MACHINE  = 2,
    HKEY_USERS          = 3,
    HKEY_CURRENT_CONFIG = 4,
}

-- Shell special-folder IDs for FilePathGetSpecialLocation()
CSIDL = {
    DESKTOP                 = 0,
    PROGRAMS                = 2,
    CONTROLS                = 3,
    PRINTERS                = 4,
    PERSONAL                = 5,
    FAVORITES               = 6,
    STARTUP                 = 7,
    RECENT                  = 8,
    SENDTO                  = 9,
    STARTMENU               = 11,
    MYDOCUMENTS             = 5,
    MYMUSIC                 = 13,
    MYVIDEO                 = 14,
    DESKTOPDIRECTORY        = 16,
    DRIVES                  = 17,
    NETWORK                 = 18,
    NETHOOD                 = 19,
    FONTS                   = 20,
    TEMPLATES               = 21,
    COMMON_STARTMENU        = 22,
    COMMON_PROGRAMS         = 23,
    COMMON_STARTUP          = 24,
    COMMON_DESKTOPDIRECTORY = 25,
    APPDATA                 = 26,
    PRINTHOOD               = 27,
    PROGRAM_FILES           = 38,
}

-- Run a batch file via cmd.exe; returns true on success.
-- Silently skips files that do not exist.
function RunBatchFile(batchPath, waitSeconds)
    if not installx.FilePathExists(batchPath) then
        installx.LogPrint("Skip missing batch file: " .. batchPath)
        return false
    end
    local command = 'cmd.exe /S /C call "' .. batchPath .. '"'
    installx.LogPrint("RunBatchFile: " .. command)
    return installx.ProcessExecute(command, false, waitSeconds or 30)
end

-- Show an error message in the UI error label.
-- Projects may override this function.
function ErrorHint(msg)
    installx.LogPrint("[Error] " .. tostring(msg))
    installx.DuiText("errortiplab", tostring(msg))
    installx.DuiVisible("errortiplab", true)
end
