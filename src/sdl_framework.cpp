#include "sdl_framework.hpp"
/*Util start*/
#ifdef USE_OPENGL
bool Explore3DEventSDL(SDL_Event* eventSDL,Camera& camera,
	float stepPos, float stepAngle, float stepFov)
{
	if (eventSDL->type != SDL_KEYDOWN && eventSDL->type != SDL_KEYUP) return false;
	
	SDL_Scancode scancode = eventSDL->key.keysym.scancode;
	bool flag = true;
	float dt = 1.f;
	glm::vec3 dpos(glm::vec3(0));
	glm::vec3 dangle(glm::vec3(0));
	float dfov = 0.f;
	glm::vec3 forward(
		cos(camera.angle.p) * cos(camera.angle.y),
		sin(camera.angle.p),
		cos(camera.angle.p) * sin(camera.angle.y));
	glm::vec3 up(sin(camera.angle.r), cos(camera.angle.r), 0.f);
	up = up - forward* glm::dot(forward, up) / glm::dot(forward, forward);
	glm::vec3 right = glm::normalize(glm::cross(forward, up)); // the right direction from object
	if (eventSDL->type == SDL_KEYDOWN)
	{
		switch(scancode)
		{
		// position
		case SDL_SCANCODE_W: // w is go forward, default is goto -z
			dpos += dt * stepPos * forward;
			break;
		case SDL_SCANCODE_A: 
			dpos -= dt * stepPos * right;
			break;
		case SDL_SCANCODE_S:
			dpos -= dt * stepPos * forward;
			break;
		case SDL_SCANCODE_D:
			dpos += dt * stepPos * right;
			break;
		case SDL_SCANCODE_Q:
			dpos -= dt * stepPos * up;
			break;
		case SDL_SCANCODE_E:
			dpos += dt * stepPos * up;
			break;
		// rotate
		case SDL_SCANCODE_I:
			dangle.p += dt * stepAngle;
			break;
		case SDL_SCANCODE_J:
			dangle.y -= dt * stepAngle;
			break;
		case SDL_SCANCODE_K:
			dangle.p -= dt * stepAngle;
			break;
		case SDL_SCANCODE_L:
			dangle.y += dt * stepAngle;
			break;
		case SDL_SCANCODE_U:
			dangle.r += dt * stepAngle;
			break;
		case SDL_SCANCODE_O:
			dangle.r -= dt * stepAngle;
			break;
		// fov
		case SDL_SCANCODE_Z:
			dfov -= dt * stepFov;
			break;
		case SDL_SCANCODE_C:
			dfov += dt * stepFov;
			break;
		default:
			flag = false;
			break;
		}
	}
	if (flag)
	{
		camera.position += dpos;
		camera.angle += dangle;
		camera.fov += dfov;
		for (int i = 0; i < 3; i++)
		{
			while (camera.angle[i] > glm::radians(180.f)) 
				camera.angle[i] -= glm::radians(360.f);
			while (camera.angle[i] < glm::radians(-180.f))
				camera.angle[i] += glm::radians(360.f);
		}
		while (camera.fov > glm::radians(180.f))
			camera.fov -= glm::radians(360.f);
		while (camera.fov < glm::radians(-180.f))
			camera.fov += glm::radians(360.f);
		
	}
	return flag;
}
#endif
/*Util end*/

/*CSceneSDL start*/
CSceneSDL::CSceneSDL(CAppSDL& appSDL) :m_appSDL(appSDL)
{

}

CSceneSDL::~CSceneSDL()
{

}

void CSceneSDL::render()
{
	for (auto it = m_objects.get().begin(); it != m_objects.get().end(); it++)
	{
		for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			(*it2)->draw();
		}
	}
}
/*CSceneSDL end*/

/*CStageSDL start*/
CStageSDL::CStageSDL(CAppSDL& appSDL):m_appSDL(appSDL)
{

}

CStageSDL::~CStageSDL()
{

}

void CStageSDL::pushScene(shared_ptr<CSceneSDL> pScene)
{
	m_scenesSDL.push_back(pScene);
	m_pCurrentScene = m_scenesSDL.back().get();
}

#ifdef USE_OPENGL
void CStageSDL::pushScene(shared_ptr<CSceneGL> pScene)
{
	m_scenesGL .push_back(pScene);
	m_pCurrentScene = m_scenesGL.back().get();
}
#endif

void CStageSDL::popScene()
{
	if (!m_appSDL.enableGL())
	{
		m_scenesSDL.pop_back();
		m_pCurrentScene = m_scenesSDL.back().get();
	}
#ifdef USE_OPENGL
	else
	{
		m_scenesGL.pop_back();
		m_pCurrentScene = m_scenesGL.back().get();
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
	if (!m_appSDL.enableGL())
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
	if (!m_appSDL.enableGL())
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
	if (!m_appSDL.enableGL())
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
	m_stages.push_back(stageSDL);
	m_currentStage = stageSDL.get();
}

void CStageManegerSDL::popStage()
{
	m_stages.pop_back();
	m_currentStage = m_stages.back().get();
}

void CStageManegerSDL::handleEvent(SDL_Event& event)
{
	if (m_currentStage == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CStageManegerSDL::handleEvent stage is NULL");
		return;
	}
	m_currentStage->handleEvent(event);
}

void CStageManegerSDL::update()
{
	if (m_currentStage == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CStageManegerSDL::update stage is NULL");
		return;
	}
	m_currentStage->update();
}

void CStageManegerSDL::render()
{
	if (m_currentStage == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CStageManegerSDL::render stage is NULL");
		return;
	}
	m_currentStage->render();
}
/*CStageManagerSDL end*/

/*CAppSDL start*/
CAppSDL::CAppSDL(Uint32 flags)
{
	if(SDL_Init(flags))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "SDL_Init error: %s", SDL_GetError());
	}
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
#ifdef USE_OPENGL
	if (m_enableGl)
	{
		createGLContext();
		glViewport(0, 0, w, h);
	}
#endif
}

void CAppSDL::prepareWindow(SDL_Window* window, SDL_Renderer *renderer)
{
	releaseSDL();
	m_bOutsideWindow = true;
	m_window = window;
	m_renderer = renderer;
#ifdef USE_OPENGL
	if (m_enableGl)
	{

		createGLContext();
		int screenW, screenH;
		SDL_GetWindowSize(m_window, &screenW, &screenH);
		glViewport(0, 0, screenW, screenH);
	}
#endif
}

void CAppSDL::prepareGL(int swap_interval, int major_version, 
	int minor_version,int context_profile, bool enable)
{
	enableGL(enable);
	releaseGL();
	//createGLContext(); //must create window before prepareGL 
	int ret;
	ret = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major_version);
	if (ret) SDL_LogError(SDL_LOG_CATEGORY_ASSERT, 
		"SDL_GL_SetAttribute error: %s", SDL_GetError());
	ret = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor_version);
	if (ret) SDL_LogError(SDL_LOG_CATEGORY_ASSERT, 
		"SDL_GL_SetAttribute error: %s", SDL_GetError());
	ret = SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, context_profile);
	if (ret) SDL_LogError(SDL_LOG_CATEGORY_ASSERT, 
		"SDL_GL_SetAttribute error: %s", SDL_GetError());
	ret = SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	ret = SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	SDL_GL_SetSwapInterval(swap_interval);
}

void CAppSDL::prepareStageManager(shared_ptr<CStageManegerSDL> stageManger)
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

void CAppSDL::createGLContext()
{
	if (m_window && m_glContext==NULL)
	{
		m_glContext = SDL_GL_CreateContext(m_window);
#ifdef _PSV
#ifdef USE_OPENGL
	vglInit(0x800000);
#endif
#else
#ifdef __GLEW_H__
		GLenum glewError = glewInit(); // if using glew, must glew init, or c++ error will occured
		if (glewError != GLEW_OK)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ASSERT,
				"Error initializing GLEW! %s\n", glewGetErrorString(glewError));
		}
#endif
#endif
		SDL_GL_SwapWindow(m_window);
	}
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

#ifdef _WEB
#include <emscripten.h>
CAppSDL* g_appsdl = NULL;
void sdl_loop()
{
	if(!g_appsdl) return;
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		g_appsdl->handleEvent(event);
	}
	g_appsdl->update();
	g_appsdl->render();
	g_appsdl->m_lastRenderTicks = SDL_GetTicks();
}
#endif

void CAppSDL::run()
{
	if (!m_stageManager)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CAppSDL::run() m_stageManager is NULL");
		return;
	}
#ifdef _WEB
	g_appsdl = (CAppSDL*)this;
	emscripten_set_main_loop(sdl_loop, -1, -1);
#else
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
#endif
}

bool CAppSDL::enableGL()
{
	return m_enableGl;
}

bool CAppSDL::enableGL(bool enable)
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