#define GLEW_STATIC
#include<GL/glew.h>
#ifdef _LINUX
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include <iostream>
#include "sdl_framework.hpp"

#ifdef _WIN32
#ifdef _DEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

class CSimpleScene :public CSceneSDL
{
public:
	CSimpleScene(CAppSDL& appSDL):CSceneSDL(appSDL)
	{
		CCircleSDL* circle = new CCircleSDL(appSDL);
		circle->create(30, { 100,0,100,255 });
		m_pObjects.pushObject(circle);
	}

	void handleEvent(void *e)
	{
		int x, y;
		SDL_GetMouseState(&x, &y);
		m_pObjects.atObject(0)->moveTo(static_cast<float>(x),  static_cast<float>(y));
	}

	~CSimpleScene()
	{
		delete m_pObjects.atObject(0);
	}
};

int main(int argc, char * argv[])
{
	SDL_Log("This is a simle demo using sdl framework");

	CAppSDL app;
	app.prepareWindow("sdl circle demo", 800, 600);
	app.prepareGL(); 
	CStageManegerSDL simple_manager(app);
	shared_ptr<CStageSDL> simple_stage = make_shared<CStageSDL> (CStageSDL(app));
	shared_ptr<CSimpleScene> simple_scene =shared_ptr<CSimpleScene>(new CSimpleScene(app));
	simple_stage.get()->pushScene(simple_scene);
	simple_manager.pushStage(simple_stage);
	app.prepareStageManager(&simple_manager);
	app.setBackground(0xff, 0xc0, 0xcb);
	app.setFps(60);
	app.run();
	SDL_Log("sdl end");
#if defined(_WIN32) && defined(_DEBUG) 
	_CrtCheckMemory();
#endif
	return 0;
}