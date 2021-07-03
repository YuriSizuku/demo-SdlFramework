CMAKELISTS_PATH=./../
sudo apt-get -y install gcc gdb make cmake git
sudo apt-get -y install libsdl2-dev libgl1-mesa-dev libglew-dev libglm-dev
if [ ! -d "$CMAKELISTS_PATH/externlib" ]; then
    mkdir $CMAKELISTS_PATH/externlib
fi
if [ ! -d "$CMAKELISTS_PATH/externlib/stb" ]; then
    git clone https://github.com/nothings/stb.git $CMAKELISTS_PATH/externlib/stb
fi