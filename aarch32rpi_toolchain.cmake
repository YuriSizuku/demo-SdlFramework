# This is a toolchain file for cross compiling rashpberry pi arm32 elf 
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
set(DCMAKE_C_FLAGS "--sysroot=${CMAKE_SYSROOT}")
set(DCMAKE_CXX_FLAGS "--sysroot=${CMAKE_SYSROOT}")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath=//usr/lib/arm-linux-gnueabihf/:/opt/vc/lib/")