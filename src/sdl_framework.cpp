#include "sdl_framework.hpp"
/*CStageSDL start*/
CStageSDL::CStageSDL(CAppSDL& appSDL):m_appSDL(appSDL) {}

CStageSDL::~CStageSDL()
{

}

void CStageSDL::pushObject(void* object)
{
	if (!m_appSDL.enableGl()) m_pObjects2DSDL.push_back(static_cast<CObject2DSDL*>(object));
	else m_pSceneGL = static_cast<CSceneGL*>(object);
}

void CStageSDL::popObject()
{
	if(!m_appSDL.enableGl()) m_pObjects2DSDL.pop_back();
}

void CStageSDL::handleEvent(SDL_Event& event)
{

}

void CStageSDL::update()
{

}

void CStageSDL::render()
{
	if (!m_appSDL.enableGl())
	{
		for (auto it=m_pObjects2DSDL.begin(); it!=m_pObjects2DSDL.end(); it++)
		{
			(*it)->draw();
		}
	}
	else
	{
		if (m_pSceneGL) m_pSceneGL->draw();
		else SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CStageSDL::render can not render as m_pSceneGL is NULL");
	}

}
/*CStageSDL end*/

/*CStageManagerSDL start*/
CStageManegerSDL::CStageManegerSDL(CAppSDL& appSDL) :m_appSDL(appSDL) 
{
	m_pCurStage = NULL;
}

CStageManegerSDL::~CStageManegerSDL()
{

}

void CStageManegerSDL::pushStage(CStageSDL* stageSDL)
{
	m_pStages.push_back(stageSDL);
	m_pCurStage = stageSDL;
}

void CStageManegerSDL::popStage()
{
	m_pStages.pop_back();
	m_pCurStage = m_pStages.back();
}

void CStageManegerSDL::handleEvent(SDL_Event& event)
{
	m_pCurStage->handleEvent(event);
}

void CStageManegerSDL::update()
{
	m_pCurStage->update();
}

void CStageManegerSDL::render()
{
	m_pCurStage->render();
}
/*CStageManagerSDL end*/

/*CAppSDL start*/
CAppSDL::CAppSDL()
{
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

void CAppSDL::prepareGL(SDL_GLContext glContext)
{
	releaseGL();
	m_glContext = glContext;
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

SDL_GLContext& CAppSDL::getGLContext()
{
	return m_glContext;
}

void CAppSDL::releaseSDL()
{
	if (m_renderer) SDL_DestroyRenderer(m_renderer);
	if (m_window) SDL_DestroyWindow(m_window);
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
	if (event.type == SDL_QUIT)
	{
		stop();
		return;
	}
	m_stageManager->handleEvent(event);
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

	if (m_fps) // restrict the max fps
	{
		Uint32 currentTicks = SDL_GetTicks();
		Uint32 frame_interval = 1000 / m_fps;
		Uint32 passed_interval = currentTicks - m_lastRenderTicks;
		if (passed_interval < frame_interval)
		{
			SDL_Delay(frame_interval - passed_interval);
		}
	}

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
	m_lastRenderTicks = SDL_GetTicks();
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
	}
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

map<string, void*>& CAppSDL::getGlobalInfo()
{
	return m_info;
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