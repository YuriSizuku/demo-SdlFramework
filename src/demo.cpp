#define GLEW_STATIC
#include<GL/glew.h>
#include <SDL.h>
#include <iostream>
#include "sdl_framework.hpp"
int main(int argc, char * argv[])
{
	SDL_Log("This is a simle demo using sdl framework");
	CAppSDL app;
	app.prepareWindow("sdl demo", 800, 600);
	app.prepareGL(); 
	app.setBackground(0xff, 0xc0, 0xcb);
	app.setFps(60);
	CCircleSDL circle(app);
	circle.createCircle(30, { 100,100,100,255 });
	app.pushObjectSDL2D((CObject2DSDL*)&circle);
	app.run();
	return 0;
}