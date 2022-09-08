BUILD_PATH=./../build_psv
CMAKELISTS_PATH=./../
SYSROOT=

cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
    -G "Unix Makefiles" \
    -DCMAKE_SYSTEM_NAME=PSV \
pushd  $BUILD_PATH
make -f $BUILD_PATH/Makefile circle_danmaku
popd