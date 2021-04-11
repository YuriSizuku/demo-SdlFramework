#define GLEW_STATIC
#include<GL/glew.h>
#ifdef _LINUX
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include <iostream>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include "sdl_framework.hpp"
#include "physics_object.hpp"

#if(defined(_WIN32) || defined(_WIN64))
#ifdef _DEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#else
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")
#endif
#endif

enum CIRCLE_TYPE {
	PLAYER,
	ENEMY,
	BULLENT,
	HUD,
};

class CCircleDanmaku :public CSingleTextureSDL, public CPhysicsRagidCircle // the basic circle of the obejct
{
private:
	Uint32 m_onDestoryTicks=0;
	Uint32 m_destoryInterval;
public:
	int m_health = 0; // heath=0 show destory, health<0 delete object
	int m_ownerType = PLAYER, m_ownerID =0; // the bullet onwer, fire out by player or enemy

	CCircleDanmaku(CAppSDL& appSDL, shared_ptr<SDL_Texture> texture) : CSingleTextureSDL(appSDL, texture)
	{
		m_destoryInterval = 300;
		int screenW, screenH;
		SDL_GetWindowSize(m_appSDL.getWindow(), &screenW, &screenH);
		setScreen(screenW, screenH);
	}

	void setRadius(float r)
	{
		m_r = r;
		m_renderRect.w = static_cast<int>(m_r * 2);
		m_renderRect.h = static_cast<int>(m_r * 2);
	}

	void drawExist()
	{
		SDL_RenderCopy(m_appSDL.getRenderer(), m_texture.get(), NULL, &m_renderRect);
		if (m_type != BULLENT) // draw the arrow
		{
			int x1 = static_cast<int>(round(m_x));
			int y1 = static_cast<int>(round(m_y));
			int x2 = static_cast<int>(round(m_x + m_r * cos(m_theta)));
			int y2 = static_cast<int>(round(m_y + m_r * sin(m_theta)));
			SDL_RenderDrawLine(m_appSDL.getRenderer(), x1, y1, x2, y2);
			SDL_RenderDrawLine(m_appSDL.getRenderer(), x1, y1, x2 + 1, y2 + 1);
			SDL_RenderDrawLine(m_appSDL.getRenderer(), x1, y1, x2 - 1, y2 - 1);
		}
	}

	void drawDestory(Uint32 passedTicks)
	{
         // the destory anime
		int  n = 8;
		float th, l, r;
		r = m_r / 2.f;
		m_renderRect.w = static_cast<int>(2 * r);
		m_renderRect.h = static_cast<int>(2 * r);
		m_renderRect.x = static_cast<int>(m_x - r);
		m_renderRect.y = static_cast<int>(m_y - r);
		SDL_RenderCopy(m_appSDL.getRenderer(), m_texture.get(), NULL, &m_renderRect);
		for (int i = 0; i < n; i++)
		{
			th = static_cast<float>(i * 2.f * M_PI) / n;
			l = r * (1.5f + static_cast<float>(passedTicks) / static_cast<float>(m_destoryInterval));
			m_renderRect.x = static_cast<int>(m_x + l * cos(th) - r);
			m_renderRect.y = static_cast<int>(m_y + l * sin(th) - r);
			SDL_RenderCopy(m_appSDL.getRenderer(), m_texture.get(), NULL, &m_renderRect);
		}
	}

	void draw()
	{
		if (!m_texture)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CCircleDanmaku::draw, m_texture is NULL!");
			return;
		}
		if (m_health>0)
		{
			drawExist();
		}
		else if(m_health==0) // in destory
		{
			if (m_type != BULLENT)
			{
				Uint32 currentTicks = SDL_GetTicks();
				m_onDestoryTicks = m_onDestoryTicks ? m_onDestoryTicks : currentTicks;
				Uint32 passedTicks = currentTicks - m_onDestoryTicks;
				if (passedTicks >= m_destoryInterval)
				{
					m_health--;
				}
				drawDestory(passedTicks);
			}
			else
			{
				m_health--; // tell
			}
		}
	}

	void move(Uint32 interval, bool loop=true)
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

	virtual ~CCircleDanmaku()
	{

	}
};

// show the HUD on screen and the information
class CCircleHUD :public CObject2DSDL
{
private:
	CCircleSDL *m_pTrueCircle, *m_pFalseCircle;
	int m_x, m_y;
public:
	int m_health=0, m_totalHealth=3, m_defeated=0;
	float m_radius;
	CCircleHUD(CAppSDL& appsdl) : CObject2DSDL(appsdl)
	{
		int screenW, screenH;
		SDL_GetWindowSize(m_appSDL.getWindow(), &screenW, &screenH);
		m_radius = static_cast<float>(screenW) / 100;
		m_pTrueCircle = new CCircleSDL(appsdl, m_radius, { 0,255,0,255 });
		m_pFalseCircle = new CCircleSDL(appsdl, m_radius, { 255, 0,0, 255 });
		moveTo(static_cast<float>(screenW - 4 * m_radius), static_cast<float>(2 * m_radius));
	}

	void draw()
	{
		SDL_Rect rect = { m_x, m_y,
			static_cast <int>(2 * m_radius),  
			static_cast <int>(2 * m_radius)};
		for (int i = 0; i < m_totalHealth; i++)
		{
			auto texture = i < m_health ? m_pTrueCircle->getTexture() : m_pFalseCircle->getTexture();
			rect.x = m_x - static_cast<int>(i * 2 * m_radius);
			SDL_RenderCopy(m_appSDL.getRenderer(), texture.get(), NULL, &rect);
		}
	}

	void moveTo(float rightx, float righty)
	{
		m_x = static_cast<int>(rightx);
		m_y = static_cast<int>(righty);
	}

	~CCircleHUD()
	{
		delete m_pTrueCircle;
		delete m_pFalseCircle;
	}
};

class CDanmakuScene :public CSceneSDL // the danmaku game code
{
private:
	int m_winFlag = 0;
	Uint32 m_startTicks = 0;
	Uint32 m_lastEnemyFireTicks = 0, m_lastAddEnemyTicks=0;
	CCircleSDL *m_pCirclePlayer, *m_pCircleEnemy;
	CCircleSDL *m_pCircleBulletPlayer, *m_pCircleBulletEnemy;
public:
	float m_radiusEnemy, m_radiusPlayer, m_radiusBullet;
	int m_playerHealth;
	int m_maxEnemy;
	float m_enemyFireInterval, m_enemyFireRate;
	float m_moveSpeed, m_bulletSpeed, m_rotateSpeed; // move angle in one step 
public:
	CDanmakuScene(CAppSDL& appSDL) :CSceneSDL(appSDL)
	{
		// init paramers
		m_maxEnemy = 50;
#ifdef _DEBUG
		m_playerHealth = 5;
#else
		m_playerHealth = 5;
#endif
		m_enemyFireInterval = 1000;
		m_enemyFireRate = 0.7f;
		m_moveSpeed = 150.f; //pixel velocity in a second
		m_bulletSpeed = 300.f;
		m_rotateSpeed = static_cast<float>(30 * M_PI / 180);

		m_radiusEnemy = 15.f;
		m_radiusPlayer = 15.f;
		m_radiusBullet = 7.f;
		m_pCirclePlayer = new CCircleSDL(m_appSDL, m_radiusPlayer, { 0x00, 0xff, 0xff, 0xff });
		m_pCircleEnemy = new CCircleSDL(m_appSDL, m_radiusEnemy, { 0xff, 0x00, 0x00, 0xff });
		m_pCircleBulletPlayer = new CCircleSDL(m_appSDL, m_radiusBullet, { 0xff, 0x00, 0xff, 0xff });
		m_pCircleBulletEnemy = new CCircleSDL(m_appSDL, m_radiusBullet, { 0xff, 0xff, 0x00, 0xff });
	}

	// init functions
	void initPlayers()
	{
		int screenW, screenH;
		SDL_GetWindowSize(m_appSDL.getWindow(), &screenW, &screenH);

		// init player
		auto p = shared_ptr<CCircleDanmaku>(new CCircleDanmaku(m_appSDL, m_pCirclePlayer->getTexture()));
		p->setRadius(m_radiusPlayer);
		p->moveTo(static_cast<float>(screenW) / 4.f, static_cast<float>(screenH) / 2.f);
		p->m_health = m_playerHealth;
		p->m_id = 0;
		p->m_type = PLAYER;
		m_objects.pushObject(p, PLAYER);
	}
	void initEnemys()
	{
		int screenW, screenH;
		SDL_GetWindowSize(m_appSDL.getWindow(), &screenW, &screenH);
		srand((unsigned int)time(NULL));
		rand();
		for (int i = 0; i < 3; i++)
		{
			float v = 0.f, vtheta = 0.f, x, y;
			x = 0.75f * static_cast<float>(screenW);
			y = 0.5f * static_cast<float>(screenH);
			v = m_moveSpeed;
			vtheta = static_cast<float>(rand() * (2.f * M_PI / RAND_MAX));
			//SDL_Log("Init object %d, (%f, %f) v=%f v_theta=%f", i + 1, x, y, v, v_theta * 180.f / M_PI);
			addEnemy(x, y, v, vtheta);
		}
	}
	void initObjects()
	{
		auto p = shared_ptr<CCircleHUD>(new CCircleHUD(m_appSDL));
		p->m_totalHealth = m_playerHealth;
		m_objects.pushObject(p, HUD);
		initPlayers();
		initEnemys();
		m_startTicks = SDL_GetTicks();
		m_winFlag = 0;
	}

	// fire a bullet from the object p
	void fireBullet(CCircleDanmaku *p) 
	{
		auto texture = p->m_type == PLAYER ?
			m_pCircleBulletPlayer->getTexture() :
			m_pCircleBulletEnemy->getTexture();
		auto pb = shared_ptr<CCircleDanmaku>(new CCircleDanmaku(m_appSDL, texture));
		pb->setRadius(m_radiusBullet);
		pb->m_health = 1;
		pb->m_vx = m_bulletSpeed * cos(p->m_theta);
		pb->m_vy = m_bulletSpeed * sin(p->m_theta);
		pb->m_x = p->m_x + (p->m_r + 1.1f * pb->m_r) * cos(p->m_theta);
		pb->m_y = p->m_y + (p->m_r + 1.1f * pb->m_r) * sin(p->m_theta);
		pb->m_id = static_cast<int>(m_objects.get()[BULLENT].size() + 1);
		pb->m_type = BULLENT;
		pb->m_ownerType = p->m_type; pb->m_ownerID = p->m_ownerID;
		m_objects.pushObject(pb, BULLENT);
	}
	void addEnemy(float x, float y, float v, float vtheta)
	{
		auto p = shared_ptr<CCircleDanmaku>(new CCircleDanmaku(m_appSDL, m_pCircleEnemy->getTexture()));
		p->setRadius(m_radiusEnemy);
		p->moveTo(x, y);
		p->m_vx = v * cos(vtheta);
		p->m_vy = v * sin(vtheta);
		p->m_theta = vtheta;
		p->m_omiga = static_cast<float>(M_PI / 2);
		p->m_health = 1;
		p->m_id = static_cast<int>(m_objects.get()[ENEMY].size());
		p->m_type = ENEMY;
		m_objects.pushObject(p, ENEMY);
	}

	// game result
	void onWin()
	{
		char title[100], message[256];
		int score = dynamic_cast<CCircleHUD*>(m_objects.atObject(0, HUD).get())->m_defeated;
		SDL_snprintf(title, 100, "You Win!");
		SDL_snprintf(message, 256, "Your defeated %d enemies, in %d seconds.\n"
			"Press R to restart\n"
			"WASD to move, Space fire bullet, JK rotate", score, (SDL_GetTicks()-m_startTicks)/1000);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, message, m_appSDL.getWindow());
	}
	void onLose()
	{
		char title[100], message[256];
		int score = dynamic_cast<CCircleHUD*>(m_objects.atObject(0, HUD).get())->m_defeated;
		SDL_snprintf(title, 100, "Game Over with score %d",  score);
		SDL_snprintf(message, 256, "Your defeated %d enemies, in %d seconds.\n"
			"You need to survive 5min, and defeat all enemies\n"
			"Press R to restart, WASD to move, Space fire bullet, JK rotate", score, (SDL_GetTicks() - m_startTicks) / 1000);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, message, m_appSDL.getWindow());
	}
	
	// update status functions
	void checkGameStatus()
	{
		// check win lose
		Uint32 currentTicks = SDL_GetTicks();
		if (currentTicks -m_startTicks>= 300*1000  && m_winFlag==0)
		{
			onWin();
			m_winFlag = 1;
		}
		bool onLoseFlag = false;
		if (m_objects.get()[PLAYER].size() <= 0)
		{
			onLoseFlag = true;
		}
		else
		{
			auto p = static_cast<CCircleDanmaku*>(m_objects.get()[PLAYER].begin()->get());
			if (p->m_health < 0) onLoseFlag = true;
		}
		
		if (onLoseFlag && m_winFlag==0)
		{
			onLose();
			m_winFlag = -1;
		}

		// check add enemy
		Uint32 interval = currentTicks - m_lastAddEnemyTicks;
		if (interval <= 3 * m_enemyFireInterval && m_objects.get()[ENEMY].size()>0) return;
		if (interval <= 1.5 * m_enemyFireInterval) return;
		int maxUpdateEnemys = static_cast<int>(m_maxEnemy - m_objects.get()[ENEMY].size());
		if (maxUpdateEnemys<=0) return;
		int screenW, screenH;
		SDL_GetWindowSize(m_appSDL.getWindow(), &screenW, &screenH);
		
		rand();
		float rand_ratio = static_cast<float>(rand())/ RAND_MAX;
		float time_ratio = 1.f + static_cast<float>(currentTicks - m_startTicks) / (1000.f*60.f);
		float radio = 1.4f;
		if (currentTicks - m_startTicks > 1000 * 30 
			&& currentTicks - m_startTicks < 1000 * 70) radio =2.f;
		int n = static_cast<int>(time_ratio* rand_ratio* radio);
#ifdef _DEBUG
		SDL_Log("%f, %f, %d", rand_ratio, time_ratio, n);
#endif 
		for (int i = 0; i < static_cast<int>(fmin(n, maxUpdateEnemys)); i++)
		{
			float x, y, v, vtheta;
			do
			{
				x = static_cast<float>(rand() * (static_cast<float>(screenW) / RAND_MAX));
				y = static_cast<float>(rand() * (static_cast<float>(screenH) / RAND_MAX));
			} while (x + y > 5.0f * (m_radiusEnemy + m_radiusPlayer));
			v = m_moveSpeed * (0.5f + static_cast<float>(rand() * 0.5f / RAND_MAX));
			vtheta = static_cast<float>(rand() * (2.f * M_PI / RAND_MAX));
			addEnemy(x, y, v, vtheta);
		}
		m_lastAddEnemyTicks = SDL_GetTicks();
	}
	void updateEnemys(Uint32 interval)
	{
		// check if enemy fire a bullet
		bool fireFlag = false;
		if (SDL_GetTicks() - m_lastEnemyFireTicks >= m_enemyFireInterval)
		{
			fireFlag = true;
			m_lastEnemyFireTicks = SDL_GetTicks();
		}
		
		int i = 0;
		srand(static_cast<unsigned int>(time(NULL)));
		rand();
		for (auto it = m_objects.get()[ENEMY].begin();
			it != m_objects.get()[ENEMY].end(); it++)
		{
			auto p1 = dynamic_cast<CCircleDanmaku*>((*it).get());
			if (p1->m_health <= 0) continue;
			p1->move(interval);
			if (fireFlag)
			{
				if(static_cast<float>(rand())/RAND_MAX <= m_enemyFireRate)
					fireBullet(p1);
			}

			// check the enemy collision with each other or player
			for (auto it2 = m_objects.get()[ENEMY].begin(); it2 != it; it2++)
			{
				auto p2 = dynamic_cast<CCircleDanmaku*>((*it2).get());
				if (p2->m_health <= 0) continue;
				p1->doCollision(p2);
			}
			int j = 0;
			for (auto it2 = m_objects.get()[PLAYER].begin(); 
				it2 != m_objects.get()[PLAYER].end(); it2++)
			{
				auto p2 = dynamic_cast<CCircleDanmaku*>((*it2).get());
				if (p2->m_health <= 0) continue;

				// turn the theta to player
				if (m_objects.get()[PLAYER].size() && i% m_objects.get()[PLAYER].size()==j)
				{
					float p1p2theta = atan2(
						p2->m_y + p2->m_vy* interval - p1->m_y, 
						p2->m_x + p2->m_vx* interval - p1->m_x); // predict the player position
					float dtheta = p1p2theta- p1->m_theta;
					if (fabs(dtheta) < M_PI)
					{
						if (dtheta * p1->m_omiga < 0) p1->m_omiga *= -1.f;
					}
					else
					{
						if (dtheta * p1->m_omiga > 0) p1->m_omiga *= -1.f;
					}
				}

				if (p1->doCollision(p2))
				{
					p2->m_health--;
				}	
			}
			p1->rotate(interval/1000.f);
			j++;
		}
		i++;
	}
	void updatePlayers(Uint32 interval)
	{
		int i = 0;
		for (auto it = m_objects.get()[PLAYER].begin();
			it != m_objects.get()[PLAYER].end(); it++)
		{
			auto p1 = dynamic_cast<CCircleDanmaku*>((*it).get());
			if (p1->m_health < 0) return;
			p1->move(interval);
			for (auto it2 = m_objects.get()[PLAYER].begin(); it2 != it; it2++)
			{
				auto p2 = dynamic_cast<CCircleDanmaku*>((*it2).get());
				p1->doCollision(p2);
			}
			i++;
		}
	}
	void updateBullets(Uint32 interval)
	{
		int screenW, screenH;
		SDL_GetWindowSize(m_appSDL.getWindow(), &screenW, &screenH);
		for (auto it = m_objects.get()[BULLENT].begin();
			it != m_objects.get()[BULLENT].end(); it++)
		{
			auto p1 = dynamic_cast<CCircleDanmaku*>((*it).get());
			if (p1->m_health <= 0) continue;
			p1->move(interval, false);
			
			// check if the bullet run out of screen
			if (p1->m_x<0 || p1->m_x>screenW || p1->m_y<0 || p1->m_vy>screenH)
			{
				p1->m_health = 0;
				continue;
			}

			// check bullet collision with enemy
			if (p1->m_ownerType != ENEMY)
			{
				for (auto it2 = m_objects.get()[ENEMY].begin(); it2 != m_objects.get()[ENEMY].end(); it2++)
				{
					auto p2 = static_cast<CCircleDanmaku*>((*it2).get());
					if (p2->m_health <= 0) continue;
					if (p1->checkCollision(p2))
					{
						p1->m_health=0;
						p2->m_health--;
						dynamic_cast<CCircleHUD*>(m_objects.atObject(p1->m_ownerID, HUD).get())->m_defeated++;
					}
				}
			}

			// check bullet collision with player
			for (auto it2 = m_objects.get()[PLAYER].begin(); it2 != m_objects.get()[PLAYER].end(); it2++)
			{
				auto p2 = dynamic_cast<CCircleDanmaku*>((*it2).get());
				if (p2->m_health <= 0) continue;
				if (p1->checkCollision(p2))
				{
					p1->m_health=0;
					p2->m_health--;
				}
			}
		}
	}
	void removeDead(int type)
	{
		for (auto it = m_objects.get()[type].begin();it != m_objects.get()[type].end();)
		{
			auto p = static_cast<CCircleDanmaku*>((*it).get());
			if (p->m_health < 0)
			{
				m_objects.removeObject(it, type);
			}
			else
			{
				it++;
			}
		}
	}

	// event function and update functions
	void handleEvent(void *event)
	{
		auto eventSDL = static_cast<SDL_Event*>(event);
		if (eventSDL->type == SDL_QUIT)
		{
			m_winFlag ? onWin() : onLose();
			return;
		}
		if (eventSDL->type != SDL_KEYDOWN && eventSDL->type != SDL_KEYUP) return;
		
		
		SDL_Scancode scancode = eventSDL->key.keysym.scancode;
		// game restart
		if (scancode == SDL_SCANCODE_R) 
		{
			releaseAllObjects();
#if  defined(_DEBUG) &&  defined(_WIN32)
		//_CrtDumpMemoryLeaks();
#endif
			initObjects();
			return;
		}		
		
		// check the player
		if (m_objects.get()[PLAYER].size() <= 0) return;
		auto pPlayer = static_cast<CCircleDanmaku*>(m_objects.get()[PLAYER].begin()->get());
		if (!pPlayer)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CDanmakuScene::handleevent, pPlayer is NULL! ");
			return;
		}
	
		// player control
		int dx = 0, dy = 0;
		float vx = pPlayer->m_vx, vy = pPlayer->m_vy, dtheta = 0.f;
		const Uint8* status = SDL_GetKeyboardState(NULL);
		if (eventSDL->type == SDL_KEYDOWN)
		{
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
		if (eventSDL->type == SDL_KEYUP)
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
		    // fire a bullet
			if (scancode == SDL_SCANCODE_SPACE)
			{
				fireBullet(pPlayer);
			}
		}
		float vtheta = atan2(vy, vx);
		float v = fabsf(vx) + fabsf(vy) != 0.f ? 1.2f*m_moveSpeed: 0.f;
		pPlayer->m_vx = v * cos(vtheta);
		pPlayer->m_vy = v * sin(vtheta);
		pPlayer->rotateTo(pPlayer->m_theta + dtheta);
	}
	void update(Uint32 currentTicks)
	{
		Uint32 interval = currentTicks - m_lastUpdateTicks;
		checkGameStatus();
		// update position and velocity by collision
		updateEnemys(interval);
		updatePlayers(interval);
		updateBullets(interval);
		// update HUD
		if (m_objects.get()[HUD].size() > 0 && m_objects.get()[PLAYER].size())
		{
			auto pHud = static_cast<CCircleHUD*>(m_objects.get()[HUD].begin()->get());
			auto pPlayer = static_cast<CCircleDanmaku*>(m_objects.get()[PLAYER].begin()->get());
			pHud->m_health = pPlayer->m_health;
		}
		// remove the object whose health is below 0
		removeDead(BULLENT);
		removeDead(ENEMY);
		removeDead(PLAYER);
		m_lastUpdateTicks = SDL_GetTicks();
	}

	void releaseAllObjects()
	{
		m_objects.get().clear();
	}

	~CDanmakuScene()
	{
		releaseAllObjects();
		delete m_pCirclePlayer;
		delete m_pCircleEnemy;
		delete m_pCircleBulletPlayer;
		delete m_pCircleBulletEnemy;
	}
};

void start()
{
	CAppSDL app;
	app.prepareWindow("circle danmaku v1.0 by devseed", 800, 600);

	// using the stage and scene after preparing the window
	auto stage_manager = shared_ptr<CStageManegerSDL>(new CStageManegerSDL(app));
	auto stage = shared_ptr<CStageSDL>(new CStageSDL(app));
	auto scene = shared_ptr<CDanmakuScene>(new CDanmakuScene(app));
	stage->pushScene(scene);
	stage_manager->pushStage(stage);
	scene->initObjects();
	app.prepareStageManager(stage_manager);
	app.setBackground(0xff, 0xc0, 0xcb);
	app.setFps(144);
	app.run();
}

int main(int argc, char* argv[])
{
	start();
#ifdef _DEBUG
	SDL_Log("game exit");
#ifdef _WIN32
	_CrtDumpMemoryLeaks();
#endif
#endif
	return 0;
}