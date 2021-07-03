@echo off
set BUILD_PATH=%~dp0\..\build_win_msvcx64pe
set CMAKELISTS_PATH=./../
cmake -B %BUILD_PATH% -S %CMAKELISTS_PATH% -G "Ninja" -DCMAKE_C_COMPILER=clang  -DCMAKE_CXX_COMPILER=clang++ -DWIN64=OFF -DCMAKE_C_FLAGS="-target x86_64-pc-windows-msvc" -DCMAKE_CXX_FLAGS="-target x86_64-pc-windows-msvc" -DWIN64=ON
pushd %BUILD_PATH%
ninja all
ninja install
popd