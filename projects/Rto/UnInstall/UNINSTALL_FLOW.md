# RTO Uninstall Flow

本文档说明 `projects/Rto/UnInstall` 的卸载执行顺序，以及哪些清理逻辑应通过 `res/UnInstall.lua` 定制。

## 目标

RTO 安装脚本会写入安装目录、注册表、快捷方式和可选 Python 环境变量。卸载逻辑按安装动作反向清理：

1. 关闭正在运行的 RTO 主进程。
2. 删除安装时写入的注册表项和值。
3. 删除桌面和开始菜单快捷方式。
4. 清理可选的 Python 环境变量和系统 `Path` 项。
5. 删除安装目录中的公共依赖目录。
6. 最后由卸载器宿主删除 `Rto` 目录和正在运行的 `UnInstall.exe`。

## 入口

卸载程序入口在 `UnInstall.cpp`：

1. 初始化 COM/OLE。
2. 设置 DuiLib 资源路径。
3. 创建 `CUninstallMainFrame` 窗口。
4. 进入 DuiLib 消息循环。

窗口初始化在 `MainFrame.cpp::Notify(DUI_MSGTYPE_WINDOWINIT)`：

1. 记录当前卸载器完整路径，用于避免递归删除时直接删除正在运行的 exe。
2. 查找 UI 控件：进度条、开始卸载按钮、完成按钮、关闭按钮。
3. 创建 `InstallLua`。
4. Debug 模式加载外部 `UnInstall.lua`，Release 模式从资源 `IDR_LUA_SCRIPT` 加载脚本。
5. 调用 Lua `OnInitialize()`。

## 初始化逻辑

`res/UnInstall.lua::OnInitialize()` 负责确定安装路径：

1. 从 `HKLM\Software\ZJU\InstallPath` 读取安装根目录。
2. 如果不存在，回退到默认路径 `C:\ZJU`。
3. 派生：
   - `dirCompany = <InstallPath>`
   - `dirExeHomeDir = <InstallPath>\Rto`
   - `dirExeFullPath = <InstallPath>\Rto\xRto.exe`
4. 写日志记录本次卸载路径。

## 用户触发流程

用户点击 `staruninstallbtn` 后：

1. 隐藏开始卸载按钮。
2. 显示进度条。
3. 禁用关闭按钮。
4. 启动进度条展开动画。
5. 动画到位后创建卸载线程，执行 `CUninstallMainFrame::UnInstall()`。

`UnInstall()` 首先弹出确认框：

1. 目标目录是当前卸载器所在目录，通常为 `<InstallPath>\Rto`。
2. 用户取消时：
   - 进度显示为取消。
   - 不调用 Lua `PreSetup()`。
   - 不删除文件目录。
3. 用户确认后才进入实际清理。

## 脚本清理阶段

确认后调用 `res/UnInstall.lua::PreSetup()`。产品相关逻辑都应放在这里定制。

当前 RTO 清理顺序：

1. 关闭进程：
   - `xRto.exe`

2. 删除注册表：
   - `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Run\Rto`
   - `HKLM\Software\ZJU\Rto`
   - `HKLM\Software\ZJU\PythonEnv`
   - `HKLM\Software\ZJU\InstallPath`
   - `HKLM\Software\Microsoft\Windows\CurrentVersion\Uninstall\Rto`

3. 清理 Python 环境：
   - 从系统 `Path` 删除 `<InstallPath>\WinPy312\python`
   - 从系统 `Path` 删除 `<InstallPath>\WinPy312\python\Scripts`
   - 删除系统环境变量 `PYTHONHOME`

4. 删除快捷方式：
   - 公共桌面 `Rto.lnk`
   - 公共开始菜单 `Programs\ZJU\Rto.lnk`
   - 公共开始菜单目录 `Programs\ZJU`

5. 删除公共依赖目录：
   - `<InstallPath>\WinPy312`
   - `<InstallPath>\Common`

脚本不会删除 `<InstallPath>\Rto`，因为卸载器自身正在该目录内运行。这个目录交给原生宿主延迟删除。

## 原生目录删除阶段

Lua `PreSetup()` 返回后，`CUninstallMainFrame::UnInstall()` 继续处理当前卸载器目录：

1. 递归统计当前目录中的子目录数量，用于估算进度。
2. 递归删除当前目录中的文件和子目录。
3. 删除文件时跳过当前正在运行的卸载器 exe。
4. 更新 UI 进度。
5. 进度达到 100 后显示完成按钮。

用户点击完成按钮或卸载完成后点击关闭时，会调用 `ClearUnInstall()`：

1. 再删除 `HKCU\Software\ZJU\RTO`。
2. 启动隐藏的 `cmd.exe`：
   - 延迟约 5 秒。
   - 执行 `rmdir /s /Q "<UnInstallDir>"`。
3. 关闭卸载窗口。

这一步用于删除仍在运行的 `UnInstall.exe` 所在目录。

## 定制规则

新增或调整 RTO 卸载动作时，优先修改 `res/UnInstall.lua`：

1. 安装脚本新增注册表项时，在 `PreSetup()` 中反向删除。
2. 安装脚本新增快捷方式时，在 `PreSetup()` 中删除。
3. 安装脚本新增环境变量或 Path 项时，在 `PreSetup()` 中清理。
4. 安装脚本新增产品外部目录时，在 `PreSetup()` 中删除。
5. 不要在 Lua 中删除 `<InstallPath>\Rto`，避免删除正在运行的卸载器。

C++ 宿主只负责：

1. 加载 UI 和 Lua 脚本。
2. 处理用户确认、进度动画和线程。
3. 调用 `OnInitialize()` / `PreSetup()`。
4. 删除卸载器自身所在目录。

## 与安装脚本的对应关系

| 安装动作 | 卸载动作 |
| --- | --- |
| 写 `HKLM\Software\ZJU\InstallPath` | 删除该值 |
| 写 `HKLM\Software\ZJU\Rto` | 删除 `Rto` 子键 |
| 写 `HKLM\Software\ZJU\PythonEnv` | 删除 `PythonEnv` 子键 |
| 写 `HKLM\...\Run\Rto` | 删除启动项 |
| 写 `HKLM\...\Uninstall\Rto` | 删除卸载注册项 |
| 创建桌面 `Rto.lnk` | 删除桌面快捷方式 |
| 创建开始菜单 `Programs\ZJU\Rto.lnk` | 删除开始菜单快捷方式和目录 |
| 可选添加 WinPy Path | 从系统 Path 移除对应项 |
| 可选设置 `PYTHONHOME` | 删除 `PYTHONHOME` |
| 解压 `WinPy312` / `Common` | 删除目录 |
| 解压 `Rto` 主目录 | C++ 宿主延迟删除自身目录 |

