# SdlFramework
My simple SDL framework for rendering, updating and event handling.



Currently the class is as below:

`CAppSDL`:  create window and renderer, contain the global information

`CStageManagerSDL`: manage the game stage, a stage can be a level or menu,  define how to change stage here

`CStageSDL`:  A stage contains many scenes in a level and handles the event if change scene.

`CSceneSDL|CSceneGL`: A scene contains many objects and determine how objects interact with each other

`CObject2DSDL|CObject3DGL`: A object used for SDL rendering or OpenGL rendering 

`CPhsicalObject`: This object contains physical information, used for physical engine



## build

I use cmake to build the demo.

### Build In windows:  

 You need either download each libraries below, or download all the requirement in [externlib](https://github.com/YuriSizuku/SdlFramework/releases/download/v0.1/externlib.7z).

[glew-2.1.0](https://sourceforge.net/projects/glew/files/glew/2.1.0/),  download the glew binary library

[glm-0.9.9](https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.7z),  download and rename `glm` to `glm-0.9.9`

[SDL-2.0.14](https://www.libsdl.org/release/SDL2-devel-2.0.14-VC.zip)  download the SDL develop binary library

create `externlib` folder and put the libraries in here , see  `CMakeLists.txt` in detail.

Use cmake  to generate visual studio sln project. Mingw is not tested.

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 14 2015" -A win32
cd ..
mkdir build_x64
cd build_x64
cmake .. -G "Visual Studio 14 2015" -A x64
```

### Build in linux: 

Install the requirement and then generate makefile.

```shell
sudo aptitude install libgl1-mesa-dev
sudo aptitude install libglew-dev glew-utils
sudo aptitude install libsdl2-dev
sudo apt install libglm-dev
mkdir build
cd build
cmake ..
make
```



## circle collision

A simulator for Perfect Elastic Collision, by conservation of momentum and energy.

Download the release demo [circle_collision](https://github.com/YuriSizuku/SdlFramework/releases/download/v0.1/circle_collision.exe)

![circle_collision](screenshot/circle_collision.gif)

## circle Danmaku

A Danmaku game whose object is all circle.

The goal in this game is to survive more than 5 minutes, and defeat the enemy as much as possible.

Download the release demo [circle_danmaku](https://github.com/YuriSizuku/SdlFramework/releases/download/v0.1/circle_danmaku.exe)

Press `R` to restart. `WASD` to move, `Space` fire bullet, `JK` rotate

![circle_danmaku_linux](screenshot/circle_danmaku_linux.png)

![circle_danmaku](screenshot/circle_danmaku.gif)

