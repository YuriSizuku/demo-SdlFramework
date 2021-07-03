@echo off
set BUILD_PATH=.\..\build_win_vsx64pe
set CMAKELISTS_PATH=.\..\
cmake -B %BUILD_PATH% -S %CMAKELISTS_PATH%  -G "Visual Studio 16 2019"  -A x64