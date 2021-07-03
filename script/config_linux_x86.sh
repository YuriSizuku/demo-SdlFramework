sudo apt-get update
sudo dpkg --add-architecture i386 
sudo apt-get -y install gcc-multilib g++multilib 
sudo apt-get -y install libsdl2-dev:i386 libgl1-mesa-dev:i386 libglew-dev:i386
sudo ln -s /usr/lib/x86_64-linux-gnu/libGLEW.so.2.1 /usr/lib/x86_64-linux-gnu/libGLEW.so # this is a error when instlal x86 glew