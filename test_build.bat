@echo off
echo Building G-Rend Roguelike...
cd /d %~dp0
C:\msys64\ucrt64\bin\make.exe clean
C:\msys64\ucrt64\bin\make.exe
if %ERRORLEVEL% EQU 0 (
    echo Build successful! Running game...
    build\roguelike.exe
) else (
    echo Build failed!
    pause
)



