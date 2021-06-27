@echo off
:: your raspberrypi root dir
set SYSROOT=//raspberrypi/chroot

mkdir %~dp0%\build_cross
cd %~dp0%\build_cross

cmake .. -G "Unix Makefiles" -DCMAKE_C_COMPILER_FORCED=ON -DCMAKE_CXX_COMPILER_FORCED=ON -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++ -DCMAKE_C_FLAGS="--sysroot=%SYSROOT%" -DCMAKE_CXX_FLAGS="--sysroot=%SYSROOT%" -DCMAKE_EXE_LINKER_FLAGS="-Wl,-rpath=//usr/lib/arm-linux-gnueabihf/:/opt/vc/lib/"

make all
make install
cd ..