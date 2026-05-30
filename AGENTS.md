# InstallXpress

Windows installer builder framework. Combines a C++ host layer, Lua scripting, and DuiLib UI to produce self-contained, single-EXE installers.

## Architecture

```
Install.lua (installation logic)
    ↓ installx.* API calls
LuaExtention.cpp (C++/Lua bridge, ~30 bindings)
    ↓ Win32 API
InstallXpress.cpp / MainFrame.cpp (process, registry, threading)
    ↓
DuiLib (UI) + 7z LZMA SDK (decompression)
```

## Build

```
mkdir build && cd build
cmake .. -DBUILD_PROJECTS=OFF
cmake --build .
```

Requires: MSVC, Windows SDK, Lua headers (`find_package(Lua)`).

Demo installers are under `projects/` and need their binary `.7z` resources present. Set `-DBUILD_PROJECTS=ON` to include them.

## Creating a New Installer Project

1. Copy `projects/TaijiMPC/Install/` as a template.
2. Edit `resource.h` — keep the conventional slot IDs from `InstallXpress.h`:

| Slot constant | ID  | Resource type  |
|---------------|-----|----------------|
| RESID_ICON    | 107 | IDI (icon)     |
| RESID_SKIN_XML| 131 | IDR (XML)      |
| RESID_LICENSE | 133 | IDR (text)     |
| RESID_SOFT1   | 134 | IDR INSTALLSOFT|
| RESID_LUA_SCRIPT | 135 | IDR LUA_SCRIPT |
| RESID_BACKGROUND | 138 | IDB (PNG)    |
| RESID_SOFT2   | 139 | IDR INSTALLSOFT|
| RESID_SOFT3   | 140 | IDR INSTALLSOFT|

3. Put your installation packages in `res/` and reference them in `Install.rc`.
4. Write `Install.lua`. The framework pre-loads these globals:
   - `HRootKey` — registry root key constants
   - `CSIDL` — shell special folder IDs
   - `RunBatchFile(path, waitSec)` — run a batch file via cmd.exe
   - `ErrorHint(msg)` — show error in UI label (override if needed)
5. Implement the required Lua callbacks:
   - `OnInitialize()` — window ready, load saved paths from registry
   - `OnButtonClick(name)` — button clicked
   - `OnSelChanged(name, selected)` — checkbox/radio changed
   - `OnUnzipProgress(notifyID, total, cur, totalBytes, curBytes)` — extraction progress
   - `PostSetup()` — called after all packages extracted
   - `QueryByKey(key)` — C++ asks Lua for a value

## Lua API Reference (`installx.*`)

### UI
| Function | Description |
|----------|-------------|
| `DuiProgress(ctrl, value, text)` | Set/get progress bar |
| `DuiText(ctrl, text\|resID)` | Set/get label text |
| `DuiVisible(ctrl, bool)` | Show/hide control |
| `DuiEnable(ctrl, bool)` | Enable/disable control |
| `DuiTabSelect(ctrl, index)` | Switch tab |
| `DuiOptionSelect(ctrl, bool)` | Check/uncheck option |
| `DuiSetBkImage(ctrl, desc)` | Set background image |
| `DuiTextColor(ctrl, color)` | Set text color (ARGB) |
| `DuiWindowPos(ctrl, x,y,w,h)` | Move/resize window |
| `DuiMessage(msg, wp, lp)` | PostMessage to window |

### File system
| Function | Description |
|----------|-------------|
| `FilePathExists(path)` | Check file/dir exists |
| `FilePathMkdir(path)` | Create directories recursively |
| `FilePathDelete(path)` | Delete file or directory |
| `FilePathCopy(src, dst, overwrite)` | Copy file |
| `FilePathChoose(title, default)` | Browse folder dialog |
| `FilePathCreateShortCut(lnk, target, workdir, desc)` | Create shell shortcut |
| `FilePathGetSpecialLocation(csidl)` | Get special folder path |
| `FilePathUnzip(resIDs, dir, skipPrefixes)` | Async parallel extraction |
| `DiskFreeSpace(path)` | Free bytes on drive |

### Process
| Function | Description |
|----------|-------------|
| `ProcessExecute(cmd, hidden, waitSec)` | Run process (MSI-aware) |
| `ProcessKill(name)` | Kill process by name |
| `RunAsAdmin()` | UAC elevation |

### Registry
| Function | Description |
|----------|-------------|
| `RegGetValue(root, path, key)` | Read registry value |
| `RegSetValue(root, path, key, value)` | Write registry value |
| `RegDeleteValue(root, path, key)` | Delete value |
| `RegDeleteKey(root, path, key)` | Delete key tree |

### System
| Function | Description |
|----------|-------------|
| `SysACP()` | Active code page (936 = Simplified Chinese) |
| `SysEnvSet(name, value)` | Set system environment variable |
| `SysPathAdd(path)` | Append to system PATH |
| `SysIsWin10OrLater()` | True if Windows 10+ |
| `SysGetArch()` | "x64" / "x86" / "arm64" |
| `SysNetworkAvailable()` | True if internet connected |
| `SysGetInstalledSoftware(name)` | Installed version or nil |

### State persistence (checkpoint/resume)
| Function | Description |
|----------|-------------|
| `StateWrite(key, value)` | Write to per-installer state file |
| `StateRead(key)` | Read from state file (nil if absent) |
| `CheckpointSet(phase)` | Record current installation phase |
| `CheckpointGet()` | Read last checkpoint phase |

State is stored in `%TEMP%\InstallXpress\<version>.state` (INI format).

### Logging
| Function | Description |
|----------|-------------|
| `LogPrint(...)` | Write to log file (trace level) |

## Key source files

| File | Purpose |
|------|---------|
| `libsrc/InstallXpress/InstallXpress.h` | Public API + resource ID conventions |
| `libsrc/InstallXpress/InstallXpress.cpp` | Win32 utilities (process, UAC, dir delete) |
| `libsrc/InstallXpress/MainFrame.cpp` | DuiLib window + async/parallel extraction |
| `libsrc/InstallXpress/LuaExtention.cpp` | All Lua C bindings + embedded base library |
| `libsrc/InstallXpress/install_base.lua` | Source for the embedded base library |
| `libsrc/InstallXpress/UnZip7z.cpp` | 7z decompression with progress callbacks |
| `projects/TaijiMPC/Install/Install.lua` | Reference installation script |
