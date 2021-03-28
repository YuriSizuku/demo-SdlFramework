#define GLEW_STATIC
#include<GL/glew.h>
#include <SDL.h>
#include <iostream>
#include "sdl_framework.hpp"

class CSimpleStage :public CStageSDL
{
public:
	CSimpleStage(CAppSDL& appSDL):CStageSDL(appSDL)
	{
		CCircleSDL* circle = new CCircleSDL(appSDL);
		circle->create(30, { 100,100,100,255 });
		m_pObjects2DSDL.push_back(circle);
	}

	void handleEvent(SDL_Event& event)
	{
		int x, y;
		SDL_GetMouseState(&x, &y);
		m_pObjects2DSDL[0]->moveTo(x, y);
	}

	~CSimpleStage()
	{
		delete m_pObjects2DSDL[0];
	}
};

int main(int argc, char * argv[])
{
	SDL_Log("This is a simle demo using sdl framework");

	CAppSDL app;
	app.prepareWindow("sdl demo", 800, 600);
	app.prepareGL(); 
	CStageManegerSDL simple_manager(app);
	CSimpleStage simple_stage(app);
	simple_manager.pushStage((CStageSDL*)&simple_stage);
	app.prepareStageManager(&simple_manager);
	app.setBackground(0xff, 0xc0, 0xcb);
	app.setFps(60);
	app.run();
	return 0;
}