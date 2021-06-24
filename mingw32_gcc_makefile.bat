@echo off
mkdir %~dp0%\build
cd %~dp0%\build
cmake .. -G "Unix Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DWIN64=OFF 
make all
make install
cd ..