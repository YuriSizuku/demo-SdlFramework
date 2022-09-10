BUILD_PATH=./../build_psv
CMAKELISTS_PATH=./../
SYSROOT=

# prepare libs
if ! [ -d ./../externlib ]; then mkdir ./../externlib; fi
if ! [ -d ./../externlib/stb ]; then git clone https://github.com/nothings/stb.git ./../externlib/stb; fi

# this need in urx:data/libshacccg.suprx, and not so stable
# if ! [ -d ./../externlib/SDL2_vitagl/ ]; then 
#     git clone https://github.com/Northfear/SDL.git ./../externlib/SDL2_vitagl
#     cmake -S ./../externlib/SDL2_vitagl/ \
#         -B ./../externlib/SDL2_vitagl/build \
#         -G "Unix Makefiles" \
#         -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake" \
#         -DVIDEO_VITA_VGL=ON \
#         -DCMAKE_BUILD_TYPE=Release
#     make -C ./../externlib/SDL2_vitagl/build
#     make -C ./../externlib/SDL2_vitagl/build install
# fi

# generate makefile
cmake -G "Unix Makefiles" -S $CMAKELISTS_PATH -B $BUILD_PATH \
    -DCMAKE_SYSTEM_NAME=PSV \
    -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake"

# make target
make -C $BUILD_PATH circle_danmaku.vpk # gl_phong_demo.vpk not supported yet