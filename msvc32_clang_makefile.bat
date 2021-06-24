@echo off
mkdir %~dp0%\build
cd %~dp0%\build
cmake .. -G "Unix Makefiles" -DCMAKE_C_COMPILER=clang  -DCMAKE_CXX_COMPILER=clang++ -DWIN64=OFF -DCMAKE_C_FLAGS="-target i686-pc-windows-msvc" -DCMAKE_CXX_FLAGS="-target i686-pc-windows-msvc"
make all
make install
cd ..