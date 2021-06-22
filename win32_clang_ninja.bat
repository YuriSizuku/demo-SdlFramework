@echo off
mkdir %~dp0%\build
cd %~dp0%\build
cmake .. -G "Ninja" -DCMAKE_C_COMPILER=clang  -DCMAKE_CXX_COMPILER=clang++ -DWIN64=OFF -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 
ninja all
ninja install
cd ..