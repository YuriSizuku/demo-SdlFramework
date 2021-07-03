@echo off
set BUILD_PATH=%~dp0\..\build_win_mingwx64pe
set CMAKELISTS_PATH=.\..\
cmake -S %CMAKELISTS_PATH% -B %BUILD_PATH%  -G "Unix Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
pushd %BUILD_PATH%
make all
make install
popd