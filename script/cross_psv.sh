BUILD_PATH=./../build_psv
CMAKELISTS_PATH=./../

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

# config path
if  [ -z "$VITASDK" ]; then VITASDK=/d/Software/env/sdk/psvsdk/; fi;
PATH=$VITASDK/bin:$PATH

# config and build project
cmake -G "Unix Makefiles" -S $CMAKELISTS_PATH -B $BUILD_PATH \
    -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake"
make -C $BUILD_PATH circle_danmaku.vpk ||\
  make -C $BUILD_PATH circle_danmaku.vpk-vpk  # for cmake higher version