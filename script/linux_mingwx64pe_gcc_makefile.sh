# configure your mingw32 for windows here
# becuase that the sturcture in mingw is not as /usr/..., don't use --sysroot here
MINGW_PATH=/d/AppExtend/msys2/mingw64
BUILD_PATH=./../build_linux_x64pe
CMAKELISTS_PATH=./../
# do not import $MINGW_PATH/x86_64-w64-mingw32, it will have conflict
FLAGS="-I$MINGW_PATH/include -L$MINGW_PATH/lib" 
# this is for the path on WSL2 is included windows path, exprot for seperate x64
export PATH=/usr/bin:$MINGW_PATH/bin:$MINGW_PATH/lib:$MINGW_PATH/x86_64-w64-mingw32/lib
cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
      -G "Unix Makefiles" \
      -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc-win32 \
      -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++-win32 \
      -DCMAKE_SYSTEM_NAME=Windows -DMINGW=ON \
      -DCMAKE_C_FLAGS="$FLAGS" \
      -DCMAKE_CXX_FLAGS="$FLAGS"
      #-DCMAKE_EXE_LINKER_FLAGS="-Wl,-rpath=$MINGW_PATH/lib:$MINGW_PATH/x86_64-w64-mingw32/lib" # no need to use -rpath hear
make -C $BUILD_PATH all
make -C $BUILD_PATH install