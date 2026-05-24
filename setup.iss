; WPStools 安装程序
#define MyAppName "WPStools"
#define MyAppVersion "1.0"
#define MyAppPublisher "wzmwayne"

[Setup]
AppId={{42548A60-2724-4C7A-8148-B57E14F1C8A9}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={localappdata}\{#MyAppName}
DefaultGroupName={#MyAppName}
UninstallDisplayIcon={app}\WPStoolsPanel.exe
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
OutputDir=.\
OutputBaseFilename=WPStools_Installer
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"

[Tasks]
Name: "desktopicon"; Description: "创建桌面快捷方式"; GroupDescription: "快捷方式："; Flags: unchecked

[Files]
Source: "dist\WPPTouchHelper.exe";     DestDir: "{app}"; Flags: ignoreversion
Source: "dist\WPStoolsPanel.exe";      DestDir: "{app}"; Flags: ignoreversion
Source: "dist\*.dll";                  DestDir: "{app}"; Flags: ignoreversion
Source: "dist\ico\*";                  DestDir: "{app}\ico"; Flags: ignoreversion
Source: "dist\platforms\*";            DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "dist\styles\*";              DestDir: "{app}\styles"; Flags: ignoreversion
Source: "dist\translations\*";         DestDir: "{app}\translations"; Flags: ignoreversion
Source: "dist\iconengines\*";          DestDir: "{app}\iconengines"; Flags: ignoreversion
Source: "dist\imageformats\*";         DestDir: "{app}\imageformats"; Flags: ignoreversion
Source: "dist\generic\*";              DestDir: "{app}\generic"; Flags: ignoreversion
Source: "dist\tls\*";                 DestDir: "{app}\tls"; Flags: ignoreversion
Source: "dist\networkinformation\*";   DestDir: "{app}\networkinformation"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\WPPTouchHelper";     Filename: "{app}\WPPTouchHelper.exe"
Name: "{autoprograms}\WPStools 配置面板";   Filename: "{app}\WPStoolsPanel.exe"
Name: "{autodesktop}\WPPTouchHelper";       Filename: "{app}\WPPTouchHelper.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\WPPTouchHelper.exe"; Description: "运行 WPPTouchHelper"; Flags: nowait postinstall skipifsilent
