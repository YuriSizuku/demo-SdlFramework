# SdlFramework
My simple SDL framework for rendering, updating and event handling.



Currently the class is as below:

`CApp`:  create window and renderer, contain the global information

`CStageManager`: manage the game stage, a stage can be a level or menu,  define how to change stage here

`CStage`:  The stage defines what objects in a level, and how the objects interactive with each other

`CObject2DSDL`: The 2D object used for SDL rendering

`CObject3DGL`: The 3D object  used for OpenGL rendering

`CPhsicalObject`: The object which contains physical information, used for physical engine



There are some of the demos I made from this small framework.

## circle collision

A simulator for Perfect Elastic Collision, by conservation of momentum and energy.

![circle_collision](screenshot/circle_collision.gif)

## circle Danmaku

A Danmaku game whose object is all circle.