# This is a toolchain file for cross compiling rashpberry pi arm32 elf 
set(CMAKE_SYSTEM_NAME Windows)
set(MINGW ON)
option(MINGW_PATH /usr/i686-w64-mingw32) # default mingw path
message("MINGW32 path" ${MINGW_PATH})
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc-win32)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++-win32)
set(CMAKE_C_FLAGS "-I${MINGW_PATH}/include -L${MINGW_PATH}/lib")
set(CMAKE_CXX_FLAGS "-I${MINGW_PATH}/include -L${MINGW_PATH}/lib")