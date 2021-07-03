@echo off
set BUILD_PATH=.\..\build_win_vsx86pe
set CMAKELISTS_PATH=.\..\
cmake -B %BUILD_PATH% -S %CMAKELISTS_PATH%  -G "Visual Studio 16 2019"  -A Win32