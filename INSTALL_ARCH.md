# TaijiMPC 安装程序架构分析

## 1. 项目概览

TaijiMPC 安装程序是一个 Win32 桌面安装器，采用 **DuiLib UI 框架 + Lua 脚本 + 7z 解压** 的三层架构：

- **C++ 宿主层**（InstallXpress 库）：窗口管理、资源加载、7z 解压线程、Lua 绑定
- **Lua 脚本层**（Install.lua）：全部安装逻辑，包括进度控制、注册表操作、进程管理
- **DuiLib UI 层**（XML 布局）：界面渲染，通过 installx.Dui* API 从 Lua 控制

入口：`Install.cpp → InstallXpress_WinMain()` → 加载 DuiLib 窗口 → 执行 Lua 脚本。

---

## 2. 目录与文件结构

```
projects/TaijiMPC/Install/
├── Install.cpp              # WinMain 入口，构造 InstallXpress_Init_t 并调用库
├── Install.lua              # 所有安装逻辑（Lua 脚本，嵌入为资源）
├── Install.rc               # Win32 资源脚本（图片、XML、7z 包、Lua 脚本）
├── resource.h               # 资源 ID 常量定义
├── Install.h / stdafx.h     # 预编译头
└── res/
    ├── InstallDialog.xml    # DuiLib 布局（中文）
    ├── InstallDialog_en.xml # DuiLib 布局（英文）
    ├── setup.png            # 安装界面主图（中文版）
    ├── setupEn.png          # 安装界面主图（英文版）
    ├── background.png       # 背景动画图（4 帧竖向拼接，600×886px）
    ├── loginlogo.png        # Logo 图
    ├── 许可协议.txt          # 中文许可协议
    ├── UserLicense.txt      # 英文许可协议
    ├── Win32.7z             # 主程序包（IDR_INSTALLSOFT1 = 134）
    └── WinPy312.7z          # Python 环境包（IDR_INSTALLSOFT2 = 139）

libsrc/InstallXpress/
├── InstallXpress.h/cpp      # 公共 API（InstallXpress_Init_t、InstallXpress_WinMain）
├── MainFrame.h/cpp          # DuiLib 主窗口、解压线程管理
├── LuaExtention.h/cpp       # Lua C 绑定（installx.* 命名空间）
├── UnZip7z.h/cpp            # 7z 解压（LZMA SDK）
└── 7ZLookInStream.h/cpp     # 资源内存流适配器（供 7z 读取嵌入资源）
```

---

## 3. 资源 ID 表（resource.h）

| 常量名               | ID  | 类型        | 内容                          |
|----------------------|-----|-------------|-------------------------------|
| IDB_RES_PNG          | 130 | PNG         | 安装界面主图（CN/EN 各一份）  |
| IDR_MAIN_XML         | 131 | XML         | DuiLib 布局文件               |
| IDB_LOGO_PNG         | 132 | PNG         | Logo                          |
| IDR_REGCONTENT1      | 133 | REGCONTENT  | 许可协议文本                  |
| IDR_INSTALLSOFT1     | 134 | INSTALLSOFT | **Win32.7z**（主程序包）      |
| IDR_LUA_SCRIPT       | 135 | LUA_SCRIPT  | Install.lua 脚本              |
| IDR_MANIFEST1        | 137 | RT_MANIFEST | 应用程序清单                  |
| IDB_RES_BACKGROUND   | 138 | PNG         | 背景动画图（4 帧竖向拼接）    |
| IDR_INSTALLSOFT2     | 139 | INSTALLSOFT | **WinPy312.7z**（Python 包）  |

---

## 4. Lua API（installx 命名空间）

### UI 控制
| 函数 | 说明 |
|------|------|
| `DuiProgress(ctrlName, value, text)` | 设置进度条值和文字 |
| `DuiSetBkImage(ctrlName, imgDesc)` | 设��背景图（支持 source 裁剪区域） |
| `DuiText(ctrlName, text/resID)` | 读写控件文字；传整数则从资源加载 |
| `DuiTextColor(ctrlName, color)` | 设置文字颜色（ARGB） |
| `DuiVisible(ctrlName, bool)` | 显示/隐藏控件；无参数则返回当前状态 |
| `DuiEnable(ctrlName, bool)` | 启用/禁用控件 |
| `DuiTabSelect(ctrlName, index)` | 切换 Tab；无参数则返回当前页 |
| `DuiOptionSelect(ctrlName, bool)` | 设置/读取 Option（单选/复选）状态 |
| `DuiWindowPos(name, x, y, w, h)` | 设置窗口位置和尺寸（name="_MainFrame"） |
| `DuiMessage(msg, wparam, lparam)` | 向主窗口 PostMessage |

### 文件操作
| 函数 | 说明 |
|------|------|
| `FilePathUnzip(resID, destDir, notifyID)` | 解压单个嵌入资源（异步） |
| `FilePathUnzip({id1,id2,...}, destDir)` | 顺序解压多个资源（异步，全部完成才触发 PostSetup） |
| `FilePathUnzip(zipPath, destDir, notifyID)` | 解压磁盘文件（异步） |
| `FilePathExists(path)` | 路径是否存在 |
| `FilePathMkdir(path)` | 递归创建目录 |
| `FilePathDelete(path)` | 删除文件或目录 |
| `FilePathCopy(src, dst, overwrite)` | 复制文件 |
| `FilePathCreateShortCut(lnk, target, workdir, desc)` | 创建快捷方式 |
| `FilePathGetSpecialLocation(csidl)` | 获取系统特殊目录（桌面、开始菜单等） |
| `FilePathChoose(title, defaultPath)` | 弹出目录选择对话框 |
| `DiskFreeSpace(path)` | 获取磁盘可用空间（字节）；path="" 则查系统盘 |

### 进程与系统
| 函数 | 说明 |
|------|------|
| `ProcessExecute(cmd, hidden, waitSec)` | 执行进程；waitSec=-1 不等待，0 不等待，>0 超时秒数 |
| `ProcessKill(exeName)` | 按进程名杀进程 |
| `RunAsAdmin()` | 请求 UAC 提权 |
| `SysACP()` | 获取系统代码页（936=中文） |
| `SysEnvSet(name, value)` | 设置系统环境变量（写 HKLM） |
| `SysPathAdd(path)` | 追加路径到系统 PATH |

### 注册表
| 函数 | 说明 |
|------|------|
| `RegGetValue(rootKey, path, key)` | 读注册表值；key="" 仅判断路径是否存在 |
| `RegSetValue(rootKey, path, key, value)` | 写注册表值（自动创建路径） |
| `RegDeleteValue(rootKey, path, key)` | 删除注册表值 |
| `RegDeleteKey(rootKey, path, key)` | 递归删除注册表键 |
| `LogPrint(...)` | 写调试日志（OutputDebugString） |

rootKey 枚举：0=HKCR, 1=HKCU, 2=HKLM, 3=HKU, 4=HKCC

---

## 5. Lua 回调函数

| 回调 | 触发时机 | 说明 |
|------|----------|------|
| `OnInitialize()` | 窗口初始化完成 | 读注册表恢复上次安装路径，检测系统语言 |
| `OnButtonClick(btnName)` | 用户点击按钮 | 处理所有按钮逻辑 |
| `OnSelChanged(btnName, isSelected)` | 复选/单选状态变化 | 用于许可协议勾选联动安装按钮 |
| `OnUnzipProgress(notifyID, totalFiles, curFile, totalSize, curSize)` | 解压进度（工作线程 PostMessage 到主线程） | 更新进度条和背景图 |
| `PostSetup()` | 所有 7z 包解压完成后 | 执行后续安装步骤（注册组件、写注册表等） |
| `QueryByKey(keyName)` | C++ 查询 Lua 状态 | 目前支持 "InstallPath" |

---

## 6. 安装流程

```
用户点击"开始安装"
    │
    ├─ RunAsAdmin()              # UAC 提权
    ├─ KillProcesses()           # 停止并卸载 HostVM/HostPy 服务，杀掉 MPC/OPCSim 进程
    ├─ CheckDiskSpace()          # 系统盘 >10MB，安装盘 >500MB
    ├─ FilePathMkdir(destDir)    # 确保安装目录存在
    │
    ├─ FilePathUnzip({134,139}, destDir)  ← 启动工作线程
    │       │
    │       ├─ 解压 Win32.7z (notifyID=134)    进度 5%~60%
    │       └─ 解压 WinPy312.7z (notifyID=139) 进度 60%~92%
    │               完成后 PostMessage → HandleCustomMessage → PostSetup()
    │
    └─ PostSetup()
            ├─ 94%  vc_redist.x64.exe /install /quiet /norestart（注册表判断跳过）
            ├─ 94%  GBDA_Install_Prereq_x86.msi（OPC DA，注册表判断跳过）
            ├─ 95%  GBDA_Install_Prereq_x64.msi（OPC DA x64，注册表判断跳过）
            ├─ 96%  TaiJiOPCSim.exe -RegServer + 手动写 COM 类别注册表
            ├─ 96%  haspdinst.exe -install（HASP 加密狗，版本 <8.31 才安装）
            ├─ 96%  install_python_env.bat（Python 虚拟环境初始化）
            ├─ 97%  写注册表（InstallPath、APPPath、Version、PythonEnv）× HKLM + WOW6432Node
            ├─ 97%  创建桌面快捷方式（TaiJiMPC5.lnk、TaiJiMPC6.lnk）
            ├─ 97%  创建开始菜单快捷方式
            ├─ 98%  HostVM/HostPy 服务 install + start
            ├─ 99%  写卸载注册表（Uninstall\TaiJiMPC5）
            └─ 100% 完成
```

---

## 7. 进度与背景图设计

### 进度分配

| 阶段 | 范围 | notifyID | 计算公式 |
|------|------|----------|---------|
| Win32.7z 解压 | 5% ~ 60% | 134 | `5 + floor(55 * curFile / totalFiles)` |
| WinPy312.7z 解压 | 60% ~ 92% | 139 | `60 + floor(32 * curFile / totalFiles)` |
| PostSetup 各步骤 | 94% ~ 100% | — | 各步骤手动设置固定值 |

完成信号（curFile=-1, totalFiles=-1）时：134→60%，139→92%，均为阶段边界值。

### 背景图切换（`IDB_RES_BACKGROUND` = res ID 138）

背景图为单张 600×886px PNG，竖向排列 4 帧（每帧约 222px）：

| 条件 | 帧区域（source） | `_image_index` |
|------|-----------------|---------------|
| 初始 | `0,0,600,221` | 0 |
| percent > 20% | `0,222,600,442` | 1 |
| percent > 50% | `0,444,600,664` | 2 |
| percent > 80% | `0,666,600,886` | 3 |

切换使用 `_image_index < N` 判断，支持进度快速跳跃时正确跨帧切换。

---

## 8. C++ 核心：MainFrame 解压机制

### 关键类型

```cpp
struct notify_msg_t {
    int nNotifyID;           // 资源 ID（134 或 139）
    int totalFileNum;        // 档案内总文件数
    int currentFileIndex;    // 当前正在解压的文件序号
    unsigned long long totalSize;
    unsigned long long currentSize;
    int isDir;
    WCHAR szFileName[MAX_PATH];
};

struct InstallXpress_Unzip_Context_t {
    std::vector<UINT> nResourceIDs;  // 多资源 ID 列表（新）
    int nResourceID;                  // 单资源 ID（兼容旧路径）
    std::wstring strZipFile;          // 文件路径（文件模式）
    std::wstring strUnzipDir;
    int nNotifyID;
};
```

### 消息流

```
工作线程（InstallThread）
    └─ InstallZip(vector<UINT>) 循环解压每个资源
            每个文件 → PostMessage(WM_INSTALLPROGRES_MSG, notifyID, notify_msg_t*)
            全部完成 → PostMessage(WM_INSTALLPROGRES_MSG, lastNotifyID, -1)

主线程（HandleCustomMessage）
    ├─ lParam != -1 → luaPtr->OnUnzipProgress(...)   # 进度回调
    └─ lParam == -1 → luaPtr->OnUnzipProgress(-1...) # 完成信号
                    → luaPtr->PostSetup()             # 触发后续安装
```

**关键设计**：多资源 ID 模式下，工作线程串行解压所有包，完成信��只在最后一包结束后发送一次，`PostSetup` 仅被调用一次。

### FilePathUnzip 三种调用形式

```lua
-- 形式1：单资源ID（兼容旧版）
installx.FilePathUnzip(134, destDir, 134)

-- 形式2：多资源ID列表（当前使用）
installx.FilePathUnzip({134, 139}, destDir)

-- 形式3：磁盘文件路径
installx.FilePathUnzip("C:\\path\\to\\file.7z", destDir, 0)
```

---

## 9. UI 控件名（DuiLib XML）

| 控件名 | 类型 | 说明 |
|--------|------|------|
| `_MainFrame` | Window | 主窗口（DuiWindowPos 目标） |
| `mainlayout` | Layout | 背景图渲染区 |
| `installlayout` | TabLayout | 0=主界面，1=许可协议 |
| `customlayout` | Layout | 自定义路径折叠区 |
| `pathedit` | Edit | 安装路径输入框 |
| `custombtn` | Button | 展开/收起自定义路径 |
| `browfilebtn` | Button | 浏览目录 |
| `starinstallbtn` | Button | 开始安装 |
| `starusebtn` | Button | 完成/启动 |
| `closebtn` | Button | 关闭（安装中禁用） |
| `lookprotbtn` | Button | 查看许可协议 |
| `protcheckbtn` | Option | 同意许可协议（联动 starinstallbtn 的启用） |
| `installprogress` | Progress | 进度条 |
| `errortiplab` | Label | 错误提示文字 |
| `titletext` | Label | 标题文字 |
| `finishlayout` | Layout | 安装完成后的选项区 |
| `sureportlayout` | Layout | 许可协议确认区 |
| `runbtn` | Option | 安装后立即运行 |
| `bootstartbtn` | Option | 开机自动启动 |
| `pyenvbtn` | Option | 将 Python 加入系统 PATH |
| `showtxbtn` | Option | 显示更新日志 |

---

## 10. 安装路径与注册表结构

### 默认安装路径

```
C:\TaijiControl\               ← _dirCompany（可自定义）
    TaiJiMPC5\TaiJiMPC.exe     ← _dirExeFullPath
    TaiJiMPC6\TaiJiMPC.exe     ← _dirExe6FullPath
    WinPy312\python\           ← Python 解释器
    WinPy312\python\Scripts\
    HostVM\HostVM.exe          ← Windows 服务（HostVM）
    HostPy\HostPy.exe          ← Windows 服务（HostPy）
    TaiJiOPCSim\bin\TaiJiOPCSim.exe
    Common\
        opc\GBDA_Install_Prereq_x86.msi
        opc\GBDA_Install_Prereq_x64.msi
        hasp\haspdinst.exe
        redist\vc_redist.x64.exe
```

### 写入的注册表键

```
HKLM\Software\TaijiControl
    InstallPath = _dirCompany
    TaiJiMPC5\APPPath = ...\TaiJiMPC5\TaiJiMPC.exe
    TaiJiMPC5\Version = 5.2.41.1
    PythonEnv\InstallPath = ...\WinPy312\python
    PythonEnv\Version = 3.12

HKLM\Software\WOW6432Node\TaijiControl   ← 32位兼容副本

HKLM\Software\Microsoft\Windows\CurrentVersion\Uninstall\TaiJiMPC5
    DisplayName = Tai-Ji MPC5
    DisplayVersion = 5.2.41.1
    Publisher = Tai-Ji Soft
    UninstallString = ...\TaiJiMPC5\UnInstall.exe
    DisplayIcon = ...\TaiJiMPC5\TaiJiMPC.exe

HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
    TaijiMPC = ...\TaiJiMPC.exe   ← 仅勾选"开机启动"时写入
```

### 跳过安装的判断键

| 组件 | 判断注册表路径 | 判断字段 |
|------|--------------|---------|
| vc_redist.x64 | `HKLM\SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64` | `Installed` |
| OPC DA x86 | `HKCR\WOW6432Node\CLSID\{13486D50-...}` | 路径存在即跳过 |
| OPC DA x64 | `HKCR\CLSID\{13486D50-...}` | 路径存在即跳过 |
| HASP 驱动 | `HKLM\SOFTWARE\Aladdin Knowledge Systems\HASP\Driver\Installer` | `DrvPkgVersion >= "8.31"` |

---

## 11. 多语言支持

- 通过 `SysACP()` 获取系统代码页：936 = 简体中文，其他 = 英文
- `_strResource` 在 `OnInitialize` 中切换为 `_strResourceCN` 或 `_strResourceEN`
- 资源文件中 LANG_CHINESE 和 LANG_NEUTRAL 各有独立的 XML 布局、PNG 图、许可协议
- UI 字符串仅有：SETUP、CHOOSE_INSTALL_PATH、LICENSE、SPACE_NOT_ENOUGH、SPACE_HINT、MKDIR_FAILED、LOADING

---

## 12. 版本信息

- 当前版本：**5.2.41.1**（`_VERSION`，写入 rc 和注册表）
- 产品名：Tai-Ji MPC
- 公司：Tai-Ji Control / Tai-Ji Soft
