#include "sdl_framework.hpp"
/*CStageSDL start*/
CStageSDL::CStageSDL(CAppSDL& appSDL):m_appSDL(appSDL)
{

}

CStageSDL::~CStageSDL()
{

}

void CStageSDL::pushScene(shared_ptr<CSceneSDL> pScene)
{
	m_pScenesSDL.push_back(pScene);
	m_pCurrentScene = m_pScenesSDL.back().get();
}

#ifdef USE_OPENGL
void CStageSDL::pushScene(shared_ptr<CSceneGL> pScene)
{
	m_pScenesGL .push_back(pScene);
	m_pCurrentScene = m_pScenesGL.back().get();
}
#endif

void CStageSDL::popScene()
{
	if (!m_appSDL.enableGl())
	{
		m_pScenesSDL.pop_back();
		m_pCurrentScene = m_pScenesSDL.back().get();
	}
#ifdef USE_OPENGL
	else
	{
		m_pScenesGL.pop_back();
		m_pCurrentScene = m_pScenesGL.back().get();
	}
#endif
}

void CStageSDL::handleEvent(SDL_Event& event)
{
	if (m_pCurrentScene == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CStageSDL::handleEvent scene is NULL");
		return;
	}
	if (!m_appSDL.enableGl())
	{
		static_cast<CSceneSDL*>(m_pCurrentScene)->handleEvent(&event);
	}
#ifdef USE_OPENGL
	else
	{
		static_cast<CSceneGL*>(m_pCurrentScene)->handleEvent(&event);
	}
#endif
}

void CStageSDL::update()
{
	Uint32 currentTicks = SDL_GetTicks();
	if (m_pCurrentScene == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CStageSDL::update scene is NULL");
		return;
	}
	if (!m_appSDL.enableGl())
	{
		static_cast<CSceneSDL*>(m_pCurrentScene)->update(currentTicks);
	}
#ifdef USE_OPENGL
	else
	{
		static_cast<CSceneGL*>(m_pCurrentScene)->update(currentTicks);
	}
#endif
}

void CStageSDL::render()
{
	Uint32 currentTicks = SDL_GetTicks();
	if (m_pCurrentScene == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CStageSDL::render scene is NULL");
		return;
	}
	if (!m_appSDL.enableGl())
	{
		static_cast<CSceneSDL*>(m_pCurrentScene)->render();
	}
#ifdef USE_OPENGL
	else
	{
		static_cast<CSceneGL*>(m_pCurrentScene)->render();
	}
#endif
}
/*CStageSDL end*/

/*CStageManagerSDL start*/
CStageManegerSDL::CStageManegerSDL(CAppSDL& appSDL) :m_appSDL(appSDL) 
{

}

CStageManegerSDL::~CStageManegerSDL()
{

}

void CStageManegerSDL::pushStage(shared_ptr<CStageSDL> stageSDL)
{
	m_pStages.push_back(stageSDL);
	m_pCurrentStage = stageSDL.get();
}

void CStageManegerSDL::popStage()
{
	m_pStages.pop_back();
	m_pCurrentStage = m_pStages.back().get();
}

void CStageManegerSDL::handleEvent(SDL_Event& event)
{
	if (m_pCurrentStage == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CStageManegerSDL::handleEvent stage is NULL");
		return;
	}
	m_pCurrentStage->handleEvent(event);
}

void CStageManegerSDL::update()
{
	if (m_pCurrentStage == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CStageManegerSDL::update stage is NULL");
		return;
	}
	m_pCurrentStage->update();
}

void CStageManegerSDL::render()
{
	if (m_pCurrentStage == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CStageManegerSDL::render stage is NULL");
		return;
	}
	m_pCurrentStage->render();
}
/*CStageManagerSDL end*/

/*CAppSDL start*/
CAppSDL::CAppSDL()
{
	m_bOutsideWindow = false;
	m_window = NULL;
	m_renderer = NULL;
	m_glContext = NULL;
	m_fps = 0;
	m_enableGl = false;
	m_lastRenderTicks = 0;
	m_background = { 0,0,0,255 };
	m_appStatus = APP_RUNNING;	
	m_stageManager = NULL;
}

CAppSDL::~CAppSDL()
{
	releaseSDL();
	releaseGL();
}

void CAppSDL::prepareWindow(string title, int w, int h, Uint32 window_flag, Uint32 renderer_flag)
{
	releaseSDL();
	m_bOutsideWindow = false;
	m_window = SDL_CreateWindow(title.c_str(),
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		w, h, window_flag);
	if(!m_window)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, 
			"SDL_CreateWindow %s error: %s", title.c_str(), SDL_GetError());
	}
	m_renderer = SDL_CreateRenderer(m_window, -1, renderer_flag);
	if (!m_renderer)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT,
			"SDL_CreateRenderer %s error: %s", title.c_str(), SDL_GetError());
	}
}

void CAppSDL::prepareWindow(SDL_Window* window, SDL_Renderer *renderer)
{
	releaseSDL();
	m_bOutsideWindow = true;
	m_window = window;
	m_renderer = renderer;
}

void CAppSDL::prepareGL(int swap_interval, int major_version, int minor_version,int context_profile)
{
	releaseGL();
	if (!m_window)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "can not create gl context before");
	}
	m_glContext = SDL_GL_CreateContext(m_window);
	int ret;
	ret = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major_version);
	if (ret) SDL_LogError(SDL_LOG_CATEGORY_ASSERT, 
		"SDL_GL_SetAttribute error: %s", SDL_GetError());
	ret = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor_version);
	if (ret) SDL_LogError(SDL_LOG_CATEGORY_ASSERT, 
		"SDL_GL_SetAttribute error: %s", SDL_GetError());
	ret = SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, swap_interval);
	if (ret) SDL_LogError(SDL_LOG_CATEGORY_ASSERT, 
		"SDL_GL_SetAttribute error: %s", SDL_GetError());
	SDL_GL_SetSwapInterval(swap_interval);
#ifdef __GLEW_H__
	GLenum glewError = glewInit(); // if using glew, must glew init, or c++ error will occured
	if (glewError != GLEW_OK)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, 
			"Error initializing GLEW! %s\n", glewGetErrorString(glewError));
	}
#endif
}

void CAppSDL::prepareStageManager(CStageManegerSDL* stageManger)
{
	m_stageManager = stageManger;
}

SDL_Window* CAppSDL::getWindow()
{
	return m_window;
}

SDL_Renderer* CAppSDL::getRenderer()
{
	return m_renderer;
}

void CAppSDL::releaseSDL()
{
	if (!m_bOutsideWindow)
	{
		if (m_renderer) SDL_DestroyRenderer(m_renderer);
		if (m_window) SDL_DestroyWindow(m_window);
	}
	m_renderer = NULL;
	m_window = NULL;
}

void CAppSDL::releaseGL()
{
	if (m_glContext) SDL_GL_DeleteContext(m_glContext);
	m_glContext = NULL;
}

void CAppSDL::handleEvent(SDL_Event& event)
{
	m_stageManager->handleEvent(event);
	if (event.type == SDL_QUIT)
	{
		stop();
		return;
	}
}

void CAppSDL::update()
{
	m_stageManager->update();
}

void CAppSDL::render()
{
	// clear screen
	if (!m_enableGl)
	{
		SDL_SetRenderDrawColor(m_renderer,
			m_background.r, m_background.g, m_background.b, m_background.a);
		SDL_RenderClear(m_renderer);
	}
#ifdef USE_OPENGL
	else
	{
		glClearColor((GLfloat)m_background.r / 255.f,
			(GLfloat)m_background.g / 255.f,
			(GLfloat)m_background.b / 255.f,
			(GLfloat)m_background.a / 255.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
#endif
	
	m_stageManager->render();

	// show the render on screen
	if (!m_enableGl)
	{
		SDL_RenderPresent(m_renderer);
	}
#ifdef USE_OPENGL
	else
	{
		SDL_GL_SwapWindow(m_window);
	}
#endif
}

void CAppSDL::run()
{
	if (!m_stageManager)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "m_stageMander is NULL");
		return;
	}
	while (m_appStatus!=APP_STOP)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			handleEvent(event);
			//update();
			//render();
		}
		update();
		render();
		if (m_fps) // restrict the max fps
		{
			float frame_interval = 1000.f / m_fps;
			Uint32 passed_interval = SDL_GetTicks() - m_lastRenderTicks;
			if (passed_interval < frame_interval)
			{
				SDL_Delay(static_cast<Uint32>(round(frame_interval)) - passed_interval);
			}
		}
		m_lastRenderTicks = SDL_GetTicks();
	}
#if(defined(_WIN32) || defined(_DEBUG))
	_CrtCheckMemory();
#endif
}

bool CAppSDL::enableGl()
{
	return m_enableGl;
}

bool CAppSDL::enableGl(bool enable)
{
	m_enableGl = enable;
	return m_enableGl;
}

void CAppSDL::setFps(int fps)
{ 
	m_fps = fps; 
}

void CAppSDL::setBackground(Uint8 r, Uint8 g, Uint8 b)
{
	m_background = { r,g,b, 255 };
}

SDL_Color& CAppSDL::getBackground()
{
	return m_background;
}

void CAppSDL::pause() 
{ 
	m_appStatus = APP_PAUSE;
}

void CAppSDL::resume() 
{ 
	m_appStatus = APP_RUNNING;
}

void CAppSDL::stop() 
{ 
	m_appStatus = APP_STOP;
}
/*CAppSDL end*/