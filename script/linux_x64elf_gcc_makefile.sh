BUILD_PATH=./../build_linux_x64elf
CMAKELISTS_PATH=./../
cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
      -G "Unix Makefiles" \
      -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++\
      -DCMAKE_C_FLAGS=-m64 -DCMAKE_CXX_FLAGS=-m64 
make -C $BUILD_PATH
# make -C $BUILD_PATH install