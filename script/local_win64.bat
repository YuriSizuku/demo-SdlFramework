@echo off
set BUILD_PATH=.\..\build_win64
set CMAKELISTS_PATH=.\..\

:: prepare libs
if not exist "%CMAKELISTS_PATH%externlib" (
    curl -fsSL https://github.com/YuriSizuku/SdlFramework/releases/download/v0.1/externlib.7z -o "%temp%\externlib.7z"
    "C:\Program Files\7-Zip\7z.exe" x "%temp%\externlib.7z" -o%CMAKELISTS_PATH%
)

:: config env
set PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin";%PATH% 

:: config and build project
cmake -B %BUILD_PATH% -S %CMAKELISTS_PATH%  -G "Visual Studio 17 2022" -A x64
msbuild %BUILD_PATH%\SdlFramework.sln -t:ALL_BUILD:rebuild -p:configuration=Release 
msbuild %BUILD_PATH%\INSTALL.vcxproj 