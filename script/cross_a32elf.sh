BUILD_PATH=./../build_a32elf
CMAKELISTS_PATH=./../
SYSROOT=

if [ -n "$(uname -a | grep Linux)" ];then
    if [ -z "$SYSROOT" ]; then
        #sudo mount -t cifs //raspberrypi/rootfs /opt/rasp -o username=pi,password=yourpw,uid=$(id -u),gid=$(id -g),forceuid,forcegid
        SYSROOT=/opt/rasp
    fi
    EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++ -Wl,-rpath-link=/usr/arm-linux-gnueabihf/lib:$SYSROOT/lib/arm-linux-gnueabihf:$SYSROOT/usr/lib/arm-linux-gnueabihf:$SYSROOT/opt/vc/lib"
else
    if [ -z "$SYSROOT" ]; then
        SYSROOT=//raspberrypi/rootfs # your raspberrypi root dir]
    fi
    EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++ -Wl,-rpath-link=$SYSROOT/lib/arm-linux-gnueabihf:$SYSROOT/usr/lib/arm-linux-gnueabihf:$SYSROOT/opt/vc/lib"
fi

echo "sysroot=$SYSROOT buildpath=$BUILD_PATH"
cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
    -G "Unix Makefiles" \
    -DCMAKE_C_COMPILER_FORCED=ON -DCMAKE_CXX_COMPILER_FORCED=ON \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc \
    -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++ \
    -DCMAKE_C_FLAGS="--sysroot=$SYSROOT"  \
    -DCMAKE_CXX_FLAGS="--sysroot=$SYSROOT" \
    -DCMAKE_EXE_LINKER_FLAGS="$EXE_LINKER_FLAGS"
pushd  $BUILD_PATH
make -f $BUILD_PATH/Makefile all
#make install
popd