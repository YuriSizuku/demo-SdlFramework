BUILD_PATH=./../build_web
CMAKELISTS_PATH=./../

# prepare libs
if ! [ -d ./../externlib ]; then mkdir ./../externlib; fi
if ! [ -d ./../externlib/stb ]; then git clone https://github.com/nothings/stb.git ./../externlib/stb; fi

# prepare path
if [ -z "$EMCSDK" ]; then EMCSDK=/d/Software/env/sdk/emsdk; fi
source "$EMCSDK/emsdk_env.sh"

# config and build project
emcmake cmake -G "Unix Makefiles" -S $CMAKELISTS_PATH -B $BUILD_PATH
make -C $BUILD_PATH circle_danmaku # gl_phong_demo.vpk not supported yet