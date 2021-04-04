# SdlFramework
My simple SDL framework for rendering, updating and event handling.



Currently the class is as below:

`CAppSDL`:  create window and renderer, contain the global information

`CStageManagerSDL`: manage the game stage, a stage can be a level or menu,  define how to change stage here

`CStageSDL`:  A stage contains many scenes in a level and handles the event if change scene.

`CSceneSDL|CSceneGL`: A scene contains many objects and determine how objects interact with each other

`CObject2DSDL|CObject3DGL`: A object used for SDL rendering or OpenGL rendering 

`CPhsicalObject`: This object contains physical information, used for physical engine



There are some of the demos I made from this small framework.

## circle collision

A simulator for Perfect Elastic Collision, by conservation of momentum and energy.

![circle_collision](screenshot/circle_collision.gif)

## circle Danmaku

A Danmaku game whose object is all circle.

The goal in this game is to survive more than 5 minutes, and defeat the enemy as much as possible.

Press `R` to restart. `WASD` to move, `Space` fire bullet, `JK` rotate

![circle_danmaku](screenshot/circle_danmaku.gif)