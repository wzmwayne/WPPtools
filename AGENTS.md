# Touchtools (WPStools) — AI 代理指南

> **重要：** 本仓库所有交互必须使用**中文**。AI 的回复、解释、代码注释、更新文档等均须使用中文。

## 项目简介

Windows 独占的 C++17/Qt6 应用，为 WPS 演示幻灯片放映提供悬浮屏幕控件。两个可执行文件通过本地命名管道通信。

## 两个可执行文件

| 目标名 | 源码目录 | 入口点 | 输出文件 | 类型 |
|--------|---------|--------|---------|------|
| `WPPTouchHelper` | `app/` | `app/main.cpp` | 后台守护进程（无窗口、无托盘图标） | `WIN32_EXECUTABLE` |
| `TouchtoolsPanel`（运行时名为 `WPStoolsPanel.exe`） | `panel/` | `panel/PanelMain.cpp` | 配置 GUI | `WIN32_EXECUTABLE` |

## 构建

```bat
build.bat
```

或手动——工具链路径硬编码在 `build.bat` 中（MSVC 14.51, WinSDK 10.0.26100.0, Qt 6.11.1 msvc2022_64）。需在 **VS 2022 开发者命令提示符** 中运行，或自行调整路径。

构建完成后自动对两个 exe 执行 `windeployqt`，产物输出到 `build/`。

### 构建注意事项
- `CMAKE_AUTOMOC ON` — 不要手动生成 MOC 文件
- 两个 `CMakeLists.txt` 均设置 `WIN32_EXECUTABLE TRUE` → 无控制台窗口（`LOG()` 调试输出到 stderr，仅调试器可见）
- `CMAKE_PREFIX_PATH` 必须指向 Qt 6 msvc2022_64

## 架构

```
WPPTouchHelper (app/)
├── main.cpp — 通过 QSharedMemory 实现单实例，QLocalServer 提供 IPC 服务
├── Config.h/.cpp — 单例，读写 <exe-dir>/touchtools.json
├── FloatingControl.h/.cpp — 两侧翻页按钮，每 500ms 轮询幻灯片状态
├── FloatingToolbar.h/.cpp — 底部工具栏（鼠标/笔/橡皮擦/退出），每 500ms 轮询
└── KeySimulator.h/.cpp — SendInput 模拟按键，CreateToolhelp32Snapshot + EnumWindows 检测 wpp.exe

TouchtoolsPanel (panel/)
├── PanelMain.cpp — 入口，创建 ConfigPanel
└── ConfigPanel.h/.cpp — 无边框暗色 UI，滑块调节参数，开机自启开关
```

## IPC（两个 exe 之间的通信）

- **传输层：** `QLocalServer`/`QLocalSocket`，命名管道 `touchtoolsPipe`
- **命令（面板 → 辅助工具）：** `"show\n"`、`"exit\n"`、`"status\n"`
- **响应（辅助工具 → 面板）：** `"active=yes"` / `"active=no"`
- **流程：** 面板保存 JSON → 发送 `"show"` → 辅助工具重载配置并重新显示控件（保持 8 秒存活）

## 配置

- JSON 文件位于 `<exe-dir>/touchtools.json`
- 惰性写入——加载时若某键缺失，则设置默认值并重写整个文件
- 面板写入文件后，通过 IPC 发送 `"show"` 命令通知辅助工具重载

## 幻灯片检测

- `KeySimulator.cpp` 通过 `CreateToolhelp32Snapshot` 轮询 `wpp.exe` PID，然后通过 `EnumWindows` 匹配窗口标题子串 `"Slide Show"` 或 `"幻灯片放映"`
- 三个控件各自独立通过 QTimer **每 500ms** 轮询
- 检测到幻灯片 → 显示并置顶窗口；未检测到 → 隐藏（IPC 触发时有 8 秒保活期）

## 按键模拟

- `sendKey()` 使用 `AttachThreadInput` + `SetForegroundWindow` + `BringWindowToTop` + `SendInput`
- 多级退出：Esc → Alt+D+Esc（×2 递增延迟）→ `TerminateProcess(wpp.exe)`

| 按钮 | 发送的按键 |
|------|-----------|
| 上一页/下一页 | `VK_LEFT` / `VK_RIGHT` |
| 鼠标模式 | `Ctrl+A` |
| 笔模式 | `Ctrl+P` |
| 橡皮擦 | `Ctrl+E` |
| 退出放映 | Esc（多级兜底） |

## 开机自启

- 使用 **PowerShell `Register-ScheduledTask`** 配合 `-AtLogOn` 触发器（不使用注册表 Run 键）
- 这样能正确设置工作目录，WPS 检测需要工作目录正确
- 在 ConfigPanel 中通过开关控制

## 死代码（查询时不要假设存在）
- `KeySimulator::highlighterMode()`（发送 `Ctrl+I`）—— 已声明但未被任何 UI 元素调用
- `bringToFront()` 在 `KeySimulator.cpp` 中 —— 文件内部函数，`sendKey()` 未使用

## 图标加载的奇怪之处
- PNG 通过 `.qrc` 编译进二进制文件，但**运行时并未从 QRC 中加载**
- `FloatingToolbar::loadIcon()` 从磁盘加载 `ico/*.png`，手动处理像素：白色像素→透明，其他所有像素→纯白色（为半透明叠加层染色）

## 安装程序

- `setup.iss` — 当前的 Inno Setup 脚本（使用 `dist/` 作为源，安装到 `%localappdata%\WPStools`）
- `install/*.iss` — 旧脚本，引用了另一个项目根目录 `e:\Users\Wayne\Documents\program\c++\WPStools\dist\`；可能已过时
- Inno Setup 需要 `dist/` 目录由 `windeployqt` 填充（DLL、platforms/、styles/、translations/ 等）

## 平台

- 仅 Windows，无可移植性层
- 使用 `windows.h`、`shellapi.h`、`tlhelp32.h` — 全程 Win32 API
- 两个可执行文件均为 x64 架构
