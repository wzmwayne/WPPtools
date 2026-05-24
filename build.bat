@echo off
chcp 65001 >nul
setlocal

set MSVC_DIR=E:\Users\Wayne\Documents\program\BuildTools\VC\Tools\MSVC\14.51.36231
set SDK_DIR=C:\Program Files (x86)\Windows Kits\10
set SDK_VER=10.0.26100.0
set QT_DIR=E:\Qt\6.11.1\msvc2022_64

set PATH=%MSVC_DIR%\bin\Hostx64\x64;%SDK_DIR%\bin\%SDK_VER%\x64;%QT_DIR%\bin;%PATH%
set INCLUDE=%MSVC_DIR%\include;%SDK_DIR%\Include\%SDK_VER%\ucrt;%SDK_DIR%\Include\%SDK_VER%\um;%SDK_DIR%\Include\%SDK_VER%\shared
set LIB=%MSVC_DIR%\lib\x64;%SDK_DIR%\Lib\%SDK_VER%\ucrt\x64;%SDK_DIR%\Lib\%SDK_VER%\um\x64

set BUILD_DIR=%~dp0build

echo === Configuring ===
cmake.exe -G "NMake Makefiles" -DCMAKE_PREFIX_PATH=%QT_DIR:/=\% -DCMAKE_BUILD_TYPE=Release -S %~dp0 -B %BUILD_DIR%
if %errorlevel% neq 0 exit /b %errorlevel%

echo === Building ===
cmake.exe --build %BUILD_DIR%
if %errorlevel% neq 0 exit /b %errorlevel%

echo === Deploying Qt DLLs ===
windeployqt --release %BUILD_DIR%\WPPTouchHelper.exe
windeployqt --release %BUILD_DIR%\WPStoolsPanel.exe

echo.
echo Build complete!
echo.
