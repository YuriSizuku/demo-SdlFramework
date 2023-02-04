#define GLEW_STATIC
#include<GL/glew.h>
#ifdef _LINUX
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include "sdl_framework.hpp"

#if defined(_MSC_VER ) && !defined(_DEBUG)
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")
#endif

#if defined(_MSC_VER ) && defined(_DEBUG)
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum CIRCLE_TYPE{
	PLAYER,
	ENEMY,
	BULLENT
};

class CCircleDanmaku :public CSingleTextureSDL, public CPhysicsRagidCircle // the basic circle of the obejct
{
public:
	SDL_Texture* m_texture;
	
	int m_health;
	int m_id;
	CIRCLE_TYPE m_type;

	CCircleDanmaku(CAppSDL& appSDL, SDL_Texture* texture): CSingleTextureSDL(appSDL)
	{
		int screenW, screenH;
		SDL_GetWindowSize(m_appSDL.getWindow(), &screenW, &screenH);
		setScreen(screenW, screenH);

		m_texture = texture;
		
		m_r = 0; 
		m_id = 0; m_health = 0;	m_type = PLAYER;
	
	}

	void setRadius(float r)
	{
		m_r = r;
		m_renderRect.w = static_cast<int>(m_r * 2);
		m_renderRect.h = static_cast<int>(m_r * 2);
	}

	void draw()
	{
		if (!m_texture)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CCircleDanmaku::draw, m_texture is NULL!");
			return;
		}
		SDL_RenderCopy(m_appSDL.getRenderer(), m_texture, NULL, &m_renderRect);
		int x1 = static_cast<int>(round(m_x));
		int y1 = static_cast<int>(round(m_y));
		int x2 = static_cast<int>(round(m_x + m_r * cos(m_theta)));
		int y2 = static_cast<int>(round(m_y + m_r * sin(m_theta)));
		SDL_RenderDrawLine(m_appSDL.getRenderer(), x1, y1, x2, y2);
		SDL_RenderDrawLine(m_appSDL.getRenderer(), x1, y1, x2+1, y2+1);
		SDL_RenderDrawLine(m_appSDL.getRenderer(), x1, y1, x2-1, y2-1);
	}

	void move(Uint32 interval, bool loop = true)
	{
		CPhysicsRagidCircle::move(static_cast<float>(interval) / 1000.f, loop);
		CSingleTextureSDL::moveTo(m_x, m_y);
	}

	void moveTo(float x, float y)
	{
		CPhysicsRagidCircle::moveTo(x, y);
		CSingleTextureSDL::moveTo(x, y);
	}

	void rotateTo(float theta, bool loop = true)
	{
		CPhysicsRagidCircle::rotateTo(theta);
	}

};

class CDanmakuScene :public CSceneSDL // the danmaku game code
{
private:
	Uint32 m_lastUpate;	
	CCircleSDL *m_pCirclePlayer, *m_pCircleEnemy, *m_pCircleBullet;
	CCircleDanmaku* m_pPlayer;
	list<CObject2DSDL*> m_pObjects2DSDL;
public:
	float m_radiusEnemy, m_radiusPlayer, m_radiusBullet;
	int m_maxEnemy, m_randomEnemyPos;
	float m_moveSpeed, m_rotateSpeed; // move angle in one step 
public:
	CDanmakuScene(CAppSDL& appSDL) :CSceneSDL(appSDL)
	{
		// init paramers
		m_lastUpate = SDL_GetTicks();
		m_radiusEnemy = 15.f;
		m_radiusPlayer = 15.f;
		m_radiusBullet = 5.f;
		
		m_maxEnemy = 50;
		m_randomEnemyPos = false;

		m_moveSpeed = 150.f; //pixel velocity in a second
		m_rotateSpeed = static_cast<float>(30 * M_PI / 180);
		
		m_pCirclePlayer = NULL; m_pCircleEnemy = NULL; m_pCircleBullet = NULL;
		m_pPlayer = NULL;
		m_pCirclePlayer = new CCircleSDL(m_appSDL, m_radiusPlayer, { 0x00, 0xff, 0xff, 0xff });
		m_pCircleEnemy = new CCircleSDL(m_appSDL, m_radiusEnemy, { 0xff, 0x00, 0x00, 0xff });
		m_pCircleBullet = new CCircleSDL(m_appSDL, m_radiusBullet, { 0xff, 0x00, 0xff, 0xff });
	}

	void initObjects()
	{
		int screenW, screenH;
		SDL_GetWindowSize(m_appSDL.getWindow(), &screenW, &screenH);
		
		// init player
		CCircleDanmaku* p = new CCircleDanmaku(m_appSDL, m_pCirclePlayer->getTexture().get());
		p->setRadius(m_radiusPlayer);
		p->moveTo(static_cast<float>(screenW) / 2.f, static_cast<float>(screenH) / 2.f);
		p->m_health = 3;
		m_pPlayer = p;
		p->m_id = 0;
		p->m_type = PLAYER;
		m_pObjects2DSDL.push_back(p);

		// init enemy
		srand((unsigned int)time(NULL));
		for (int i = 0; i < m_maxEnemy; i++)
		{
			CCircleDanmaku* p = new CCircleDanmaku(m_appSDL, m_pCircleEnemy->getTexture().get());
			p->setRadius(m_radiusEnemy);
			
			float v=0.f, v_theta=0.f, x, y;
			if (!m_randomEnemyPos)
			{
				//x = (i + 1) % int(sqrt(m_radiusEnemy)) * screenW / sqrt(m_radiusEnemy);
				//y = (i + 1) / int(sqrt(m_radiusEnemy)) * screenH / sqrt(m_radiusEnemy);
				x = screenW / 4.f; 
				y = screenH / 2.f;
				v = m_moveSpeed;
				v_theta = static_cast<float>((i + 1) * (360.f / m_maxEnemy) * M_PI / 180.f);
			}
			else
			{
				x = static_cast<float>(rand() * (static_cast<float>(screenW) / RAND_MAX));
				y = static_cast<float>(rand() * (static_cast<float>(screenH) / RAND_MAX));
				v = static_cast<float>(rand() * (m_moveSpeed/ RAND_MAX));
				v_theta = static_cast<float>(rand() * (2.f*M_PI / RAND_MAX));
				SDL_Log("Init object %d, (%f, %f) v=%f v_theta=%f", i + 1, x, y, v, v_theta * 180.f/M_PI );
			}
			
			p->moveTo(x, y);
			p->m_vx = v*cos(v_theta);
			p->m_vy = v*sin(v_theta);
			p->m_theta = v_theta;	
			p->m_omiga = static_cast<float>(M_PI / 2);
			p->m_health = 1;
			p->m_id = i + 1;
			p->m_type = ENEMY;
			m_pObjects2DSDL.push_back(p);
		}
	}

	bool checkCollision(CCircleDanmaku* p1, CCircleDanmaku* p2)
	{
		return (p2->m_x - p1->m_x) * (p2->m_x - p1->m_x) +
			(p2->m_y - p1->m_y) * (p2->m_y - p1->m_y) <=
			(p1->m_r + p2->m_r) * (p1->m_r + p2->m_r);
	}

	//collision simulation, p1 impact p2
	bool doCollision(CCircleDanmaku* p1, CCircleDanmaku* p2) 
	{
		// check if collision
		float cx, cy; // vector c , p1 center -> p2 center vector
		float lc, dcenter; // length of the (cx, cy) vector
		cx = p2->m_x - p1->m_x;
		cy = p2->m_y - p1->m_y;
		lc = sqrt(cx * cx + cy * cy);
		dcenter = p1->m_r + p2->m_r;
		if (lc > dcenter) return false;
		if (lc == 0.f)
		{
			p1->moveTo(p1->m_x, p1->m_y + dcenter);
			return true;
		};
		cx /= lc; cy /= lc; // make normalized

		// make sure tangent point not in the inner cicle
		if (dcenter - lc > 1.f)
		{
			p1->m_x = p2->m_x - 1.f * dcenter * cx;
			p1->m_y = p2->m_y - 1.f * dcenter * cy;
		}
		
		// change the coordinate x axies to p1->p2 vector
		float t;
		float v1cx, v1cy, v2cx, v2cy; // the coordinate when vector c is x axis
		float cth, v1th, v2th; // the angle of the vector c, v1, v2 start from origin x axis
		float v1cx2, v2cx2; // after collision, vertical vector not changed
		cth = atan2(cy, cx);
		v1th = atan2(p1->m_vy, p1->m_vx);
		v2th = atan2(p2->m_vy, p2->m_vx);
		t = sqrtf(p1->m_vx * p1->m_vx + p1->m_vy * p1->m_vy);
		v1cx = t * cos(cth - v1th); // the screen coordinate is inverse than the normal coordinate
		v1cy = t * sin(cth - v1th);
		t = sqrtf(p2->m_vx * p2->m_vx + p2->m_vy * p2->m_vy);
		v2cx = t * cos(cth - v2th);
		v2cy = t * sin(cth - v2th);

		// check if the ciclre can catch and impact
		if (v1cx < v2cx) return false;
		
		// collision calculate
		v1cx2 = ((p1->m_m - p2->m_m) * v1cx + 2*p2->m_m*v2cx) / (p1->m_m + p2->m_m);
		v2cx2 = ((p2->m_m - p1->m_m) * v2cx + 2*p1->m_m*v1cx) / (p1->m_m + p2->m_m);
		p1->m_vx = (v1cx2 * cx + v1cy * cy);
		p1->m_vy = (v1cx2 * cy - v1cy * cx);
		p2->m_vx = (v2cx2 * cx + v2cy * cy);
		p2->m_vy = (v2cx2 * cy - v2cy * cx);
		return true;
	}

	void handleEvent(SDL_Event& event)
	{
		if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) return;

		// game keyboard event
		int dx = 0, dy = 0;
		float vx = m_pPlayer->m_vx, vy = m_pPlayer->m_vy, dtheta = 0.f;
		SDL_Scancode scancode = event.key.keysym.scancode;
		if (event.type == SDL_KEYDOWN)
		{
			// game restart
			if (scancode == SDL_SCANCODE_R)
			{
				releaseAllObjects();
				initObjects();
			}
			// player move start
			if (scancode == SDL_SCANCODE_W || scancode == SDL_SCANCODE_UP)
				vy = -m_moveSpeed;
			if (scancode == SDL_SCANCODE_A || scancode == SDL_SCANCODE_LEFT)
				vx = -m_moveSpeed;
			if (scancode == SDL_SCANCODE_S || scancode == SDL_SCANCODE_DOWN)
				vy = m_moveSpeed;
			if (scancode == SDL_SCANCODE_D || scancode == SDL_SCANCODE_RIGHT)
				vx = m_moveSpeed;
			if (scancode == SDL_SCANCODE_J)
				dtheta = -m_rotateSpeed;
			if (scancode == SDL_SCANCODE_K)
				dtheta = m_rotateSpeed;
		}
		const Uint8* status = SDL_GetKeyboardState(NULL);
		if (event.type == SDL_KEYUP)
		{
			// player move end
			if (!status[SDL_SCANCODE_W] && !status[SDL_SCANCODE_UP] && vy < 0.f)
				vy = 0.f;
			if (!status[SDL_SCANCODE_A] && !status[SDL_SCANCODE_LEFT] && vx < 0.f)
				vx = 0.f;
			if (!status[SDL_SCANCODE_S] && !status[SDL_SCANCODE_DOWN] && vy > 0.f)
				vy = 0.f;
			if (!status[SDL_SCANCODE_D] && !status[SDL_SCANCODE_RIGHT] && vx > 0.f)
				vx = 0.f;
			if (!status[SDL_SCANCODE_J] && dtheta < 0.f)
				dtheta = 0.f;
			if (!status[SDL_SCANCODE_K] && dtheta > 0.f)
				dtheta = 0.f;
		}
		m_pPlayer->m_vx = vx;
		m_pPlayer->m_vy = vy;
		m_pPlayer->rotate(dtheta);
	}

	void handleEvent(void* event)
	{
		handleEvent(*static_cast<SDL_Event*>(event));
	}

	void update()
	{
		bool collision_flag = false;
		Uint32 interval = SDL_GetTicks() - m_lastUpate;
		
		// update position and velocity by collision
		for (auto it = m_pObjects2DSDL.begin(); it != m_pObjects2DSDL.end(); it++)
		{
			static_cast<CCircleDanmaku*>(*it)->move(interval);
			auto p1 = static_cast<CCircleDanmaku*>(*it);
			for (auto it2 = m_pObjects2DSDL.begin(); it2 != it; it2++)
			{
				auto p2 = static_cast<CCircleDanmaku*>(*it2);
				if (doCollision(p1, p2))
				{
					collision_flag = true;
				}
			}
			static_cast<CCircleDanmaku*>(*it)->rotateTo(atan2(p1->m_vy, p1->m_vx));
		}

		// calculate the sumary of the momentum and the energy
		if (collision_flag)
		{
			float Px = 0.f, Py = 0.f, E = 0.f;
			for (auto it = m_pObjects2DSDL.begin(); it != m_pObjects2DSDL.end(); it++)
			{
				float tpx = static_cast<CCircleDanmaku*>(*it)->m_vx * static_cast<CCircleDanmaku*>(*it)->m_m;
				float tpy = static_cast<CCircleDanmaku*>(*it)->m_vy * static_cast<CCircleDanmaku*>(*it)->m_m;
				Px += tpx;
				Py += tpy;
				float te = tpx * static_cast<CCircleDanmaku*>(*it)->m_vx + tpy * static_cast<CCircleDanmaku*>(*it)->m_vy;
				E += te / 2.f;
			}
			SDL_Log("in %dms, after collision, P=(%f %f), E=%f", SDL_GetTicks(), Px, Py, E);
		}
		m_lastUpate = SDL_GetTicks();
	}

	void update(Uint32 currentTicks)
	{
		update();
	}

	void render()
	{
		for (auto it = m_pObjects2DSDL.begin(); it != m_pObjects2DSDL.end(); it++)
		{
			(*it)->draw();
		}
	}

	void releaseAllObjects()
	{
		for (auto obj : m_pObjects2DSDL)
		{
			delete obj;
		}
		m_pObjects2DSDL.clear();
	}

	~CDanmakuScene()
	{
		releaseAllObjects();
		delete m_pCirclePlayer, m_pCircleEnemy, m_pCircleBullet;
	}
};

int main(int argc, char* argv[])
{
	std::cout << "circle_collision [-r(random inintial)] -n number  -v velocity" << std::endl;
	CAppSDL app;
	app.prepareWindow("circle collision", 800, 600);
	auto stage_manager = shared_ptr<CStageManegerSDL>(new CStageManegerSDL(app));
	auto stage = shared_ptr<CStageSDL>(new CStageSDL(app));
	auto scene = shared_ptr<CDanmakuScene>(new CDanmakuScene(app));
	stage->pushScene(scene);
	stage_manager->pushStage(stage);
	
	// init stage collision parameters
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0]!='-') continue;
		switch (tolower(argv[i][1]))
		{
		case 'r':
			scene->m_randomEnemyPos = true;
			break;
		case 'n':
			if (i + 1 > argc-1 && !isdigit(argv[i + 1][0]))
			{
				std::cout << "error: -n must append a number" << std::endl;
			}
			scene->m_maxEnemy = strtol(argv[i + 1], NULL, 10);
			break;
		case 'v':
			if (i + 1 > argc - 1 && !isdigit(argv[i + 1][0]))
			{
				std::cout << "error: -v must append a number" << std::endl;
			}
			scene->m_moveSpeed = strtof(argv[i + 1], NULL);
			break;
		}
	}
	
	scene->initObjects();
	app.prepareStageManager(stage_manager);
	app.setBackground(0xff, 0xc0, 0xcb);
	app.setFps(144);
	app.run();
	return 0;
}