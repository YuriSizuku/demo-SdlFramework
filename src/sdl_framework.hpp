/*
   A very simple sdl framework 
   v0.1, developed by devseed
*/

#ifndef _SDL_FRAMEWORK_H
#define _SDL_FRAMEWORK_H
#ifdef USE_OPENGL
	#ifdef _PSV
		#include <vitaGL.h>
	#else
		#define GLEW_STATIC
		#include<GL/glew.h>
		#ifdef _WIN32
		#endif
	#endif
	#include "gl_scene.hpp"
#endif
#if defined(_LINUX)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#include<string>
#include<vector>
#include<memory>
#include "sdl_object2d.hpp"
#include "data_types.hpp"
using std::vector;
using std::string;
using std::shared_ptr;

#if(defined(_WIN32) || defined(_WIN64))
#if(defined(_DEBUG) && !defined(__GNUC__))  
	#include <crtdbg.h>
	#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

#ifdef USE_OPENGL
// WASD position, QE up down, ZC fov,  R reset
// JL yaw, IK pitch, UO roll; return true if move
bool Explore3DEventSDL(SDL_Event* eventSDL, Camera& camera,
	float stepPos,float stepAngle, float stepFov);
#endif

enum AppStatus
{
	APP_RUNNING, 
	APP_PAUSE, 
	APP_STOP
};
class CObject2DSDL;
class CSceneSDL;
class CAppSDL;

// A scene contains many objects and determine how objects interact with each other
class CSceneSDL :public CScene<CMapList<shared_ptr<CObject2DSDL>>>
{
protected:
	CAppSDL& m_appSDL;
public:
	CSceneSDL(CAppSDL& appSDL);
	virtual ~CSceneSDL();
	virtual void render();
};

// the stage contains multi scenes, handles event
// must use this class after prepare window
class CStageSDL 
{
protected:
	CAppSDL& m_appSDL;
	vector<shared_ptr<CSceneSDL>> m_scenesSDL;
	void* m_pCurrentScene = nullptr;
#ifdef USE_OPENGL
	vector<shared_ptr<CSceneGL>> m_scenesGL;
#endif
public:
	CStageSDL(CAppSDL& appSDL);
	void pushScene(shared_ptr<CSceneSDL> scene);
#ifdef USE_OPENGL
	void pushScene(shared_ptr<CSceneGL> scene);
#endif
	void popScene();
	
	virtual ~CStageSDL();
	virtual void handleEvent(SDL_Event& event);
	virtual void update();
	virtual void render();
};

// the overalall control of the app, such as change current stage
// inherit this class to add game level change, such as menu
class CStageManegerSDL 
{
protected:
	CAppSDL& m_appSDL;
	CStageSDL* m_currentStage=nullptr;
	vector<shared_ptr<CStageSDL>> m_stages;
public:
	CStageManegerSDL(CAppSDL& appSDL);
	void pushStage(shared_ptr<CStageSDL> stageSDL);
	void popStage();

	virtual ~CStageManegerSDL();
	virtual void handleEvent(SDL_Event& event);
	virtual void update();
	virtual void render();
};

// one app has one window, one stageManager, multi stage, multi object
class CAppSDL 
{
private:
	Uint32 m_lastRenderTicks = 0;
	bool m_bOutsideWindow = false;
protected:
#ifdef _WEB
	friend void sdl_loop();
#endif
	SDL_Window* m_window = NULL; // for multi window
	SDL_Renderer* m_renderer = 0;
	SDL_GLContext m_glContext = {0};
	AppStatus m_appStatus = APP_RUNNING;
	SDL_Color m_background = {255,255,255,255};
	shared_ptr<CStageManegerSDL> m_stageManager = nullptr;
	int m_fps = 0;
	bool m_enableGl = false;

	void createGLContext();
	void releaseSDL();
	void releaseGL();
	void handleEvent(SDL_Event& event);
	void update(); // this is for update the pysical status
	void render();
public:	
	// init the app
	CAppSDL(Uint32 flags= SDL_INIT_EVERYTHING);
	virtual ~CAppSDL();
	void prepareWindow(string title, int w, int h,
		Uint32 window_flag = SDL_WINDOW_OPENGL, 
		Uint32 renderer_flag = SDL_RENDERER_ACCELERATED);
	void prepareWindow(SDL_Window* window, SDL_Renderer* render);
	void prepareGL(int swap_interval = 0, int major_version = 3, int minor_version = 2,
		           int context_profile= SDL_GL_CONTEXT_PROFILE_ES,
	               bool enable = true); // prepareGL should use before prepareWindow
	void prepareStageManager(shared_ptr<CStageManegerSDL> stageManger);
	SDL_Window* getWindow();
	SDL_Renderer* getRenderer();
	
	// get or set app status
	bool enableGL();
	bool enableGL(bool enable);
	void setFps(int fps);
	void setBackground(Uint8 r, Uint8 g, Uint8 b);
	SDL_Color& getBackground();
	
	void run();
	void pause();
	void resume();
	void stop();
};
#endif