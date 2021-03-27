#ifdef USE_OPENGL
#define GLEW_STATIC
#include<GL/glew.h>
#include "gl_object3d.hpp"
#endif
#include<string>
#include<vector>
#include<map>
#include<SDL.h>
#include "sdl_object2d.hpp"

#ifndef _SDL_FRAMEWORK_H
#define _SDL_FRAMEWORK_H
using std::vector;
using std::string;
using std::map;
enum AppStatus
{
	APP_RUNNING, 
	APP_PAUSE, 
	APP_STOP
};

class CObject2DSDL;
class CAppSDL
{
private:
	Uint32 m_lastRenderTicks;
protected:
	SDL_Window* m_window; // for multi window
	SDL_Renderer* m_renderer;
	SDL_GLContext m_glContext;
	vector<CObject2DSDL*> m_pObjects2DSDL; // the randering object
	map<string, void *> m_info; // the gloable information for updata
	AppStatus m_appStatus;
	int m_fps;
	SDL_Color m_background;
	bool m_enableGl;
public:	
	// init the app
	CAppSDL();
	virtual ~CAppSDL();
	void pushObjectSDL2D(CObject2DSDL* object);
	void popObjectSDL2D();
	vector<CObject2DSDL*>& getObjects2DSDL();
	map<string, void*>& getInfoMap();

	void prepareWindow(string title, int w, int h,
		Uint32 window_flag = SDL_WINDOW_OPENGL, 
		Uint32 renderer_flag = SDL_RENDERER_ACCELERATED);
	void prepareWindow(SDL_Window *window, SDL_Renderer* render);
	void prepareGL(int swap_interval=0, int major_version = 3, int minor_version = 2,
		           int context_profile= SDL_GL_CONTEXT_PROFILE_ES);
	void prepareGL(SDL_GLContext glContext);
	SDL_Window* getWindow();
	SDL_Renderer* getRenderer();
	SDL_GLContext& getGLContext();
	
	void releaseSDL();
	void releaseGL();
	
	// for event loop
	void handleEvent(SDL_Event& event);
	void update(); // this is for update the pysical status
	void render();
	void run();
	
	// change app statues
	void enableGl(bool enable);
	void setFps(int fps);
	void setBackground(Uint8 r, Uint8 g, Uint8 b);
	SDL_Color& getBackground();
	void pause();
	void resume();
	void stop();
};
#endif