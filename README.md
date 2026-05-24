# WPStools — WPS PPT 触屏辅助工具

在触屏设备上使用 WPS 演示放映幻灯片时，提供悬浮翻页控件和工具栏，无需键盘即可操作。

## 功能

- **悬浮翻页按钮** — 屏幕两侧半透明按钮，上下翻页（模拟键盘方向键）
- **底部工具栏** — 切换鼠标/笔/橡皮擦模式（Ctrl+A / Ctrl+P / Ctrl+E），一键退出放映
- **配置面板** — 无边框暗色 UI，调节透明度、尺寸、边缘距离等
- **开机自启** — 通过 Task Scheduler 实现（非注册表），工作目录自动设置
- **自动检测** — 检测到 WPS 幻灯片放映时自动显示控件，退出放映后自动隐藏
- **多级退出** — Esc → Alt+D+Esc → TerminateProcess 兜底，避免墨迹弹窗卡住

## 项目结构

```
WPStools/
├── app/                  # WPPTouchHelper（后台控件程序）
│   ├── main.cpp          # 入口：单实例、本地 IPC、加载配置
│   ├── Config.h/.cpp     # JSON 配置单例
│   ├── KeySimulator.h/.cpp  # SendInput 按键 + 幻灯片窗口检测
│   ├── FloatingControl.h/.cpp  # 侧边翻页按钮
│   └── FloatingToolbar.h/.cpp  # 底部工具栏
├── panel/                # WPStoolsPanel（配置面板）
│   ├── PanelMain.cpp     # 入口
│   └── ConfigPanel.h/.cpp  # 无边框 GUI，滑块调节参数
├── ico/                  # PNG 图标素材
│   ├── pen.png
│   ├── mouse.png
│   ├── eraser.png
│   └── exit.png
└── dist/                 # 打包输出目录
```

## 构建

### 环境要求

- Windows 10/11 x64
- Visual Studio 2022 BuildTools（MSVC 14.4+）
- Qt 6.x（msvc2022_64）
- CMake 3.16+

### 构建步骤

```bat
build.bat
```

或手动：

```bat
set INCLUDE=<MSVC include>;<SDK include>
set LIB=<MSVC lib>;<SDK lib>
set PATH=<MSVC bin>;<SDK bin>;<Qt bin>

cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
windeployqt --release build/WPPTouchHelper.exe
windeployqt --release build/WPStoolsPanel.exe
```

产物在 `build/` 下：`WPPTouchHelper.exe` + `WPStoolsPanel.exe`

## 使用

1. 运行 `WPPTouchHelper.exe`（托盘无图标，后台常驻）
2. 打开 WPS 演示并开始放映幻灯片
3. 屏幕两侧出现翻页按钮，底部出现工具栏
4. 运行 `WPStoolsPanel.exe` 打开配置面板调节参数

### 按键映射

| 操作 | 快捷键 |
|------|--------|
| 上一页 | ← |
| 下一页 | → |
| 鼠标模式 (Ctrl+A) | 工具栏按钮 |
| 笔模式 (Ctrl+P) | 工具栏按钮 |
| 橡皮擦 (Ctrl+E) | 工具栏按钮 |
| 退出放映 | Esc (多级重试) |

## 开源许可

版权所有 (C) 2026 Wayne

本程序是自由软件：您可以依据自由软件基金会发布的 GNU 通用公共许可证（**GPLv3 或任何更高版本**）重新分发或修改它。

本程序分发时希望它有用，但**不提供任何担保**；甚至没有适销性或特定用途适用性的默示担保。详见 [LICENSE](LICENSE)。

## Qt 框架

本程序使用 Qt 框架。Qt 是 The Qt Company 的商标。
从以下地址获取 Qt 源码：<https://download.qt.io/>
