#define GLEW_STATIC
#include<GL/glew.h>
#include <SDL.h>
#include <iostream>
#include "sdl_framework.hpp"

enum CIRCLE_TYPE {
	PLAYER,
	ENEMY,
	BULLENT
};

class CCircleDanmaku :public CRagidSDL // the basic circle of the obejct
{
public:
	SDL_Texture* m_texture;
	float m_r; // radius of circle

	int m_health; // heath=0 show destory, health<0 delete object

	CCircleDanmaku(CAppSDL& appSDL, SDL_Texture* texture) : CRagidSDL(appSDL)
	{
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
		if (m_health>0)
		{
			SDL_RenderCopy(m_appSDL.getRenderer(), m_texture, NULL, &m_renderRect);
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
		else if(m_health==0) // in destory
		{
			if (m_type != BULLENT)
			{

			}
			m_health--;
		}
	}
};

class CDanmakuStage :public CStageSDL // the danmaku game code
{
private:
	Uint32 m_lastUpate;
	CCircleSDL* m_pCirclePlayer, * m_pCircleEnemy, * m_pCircleBullet;
public:
	float m_radiusEnemy, m_radiusPlayer, m_radiusBullet;
	int m_maxEnemy, m_randomEnemyPos;
	float m_moveSpeed, m_bulletSpeed, m_rotateSpeed; // move angle in one step 
public:
	CDanmakuStage(CAppSDL& appSDL) :CStageSDL(appSDL)
	{
		// init paramers
		m_lastUpate = SDL_GetTicks();
		m_radiusEnemy = 15.f;
		m_radiusPlayer = 15.f;
		m_radiusBullet = 7.f;

		m_maxEnemy = 20;
		m_randomEnemyPos = false;

		m_moveSpeed = 150.f; //pixel velocity in a second
		m_bulletSpeed = 300.f;
		m_rotateSpeed = static_cast<float>(30 * M_PI / 180);

		m_pCirclePlayer = NULL; m_pCircleEnemy = NULL; m_pCircleBullet = NULL;

		m_pCirclePlayer = new CCircleSDL(m_appSDL, m_radiusPlayer, { 0x00, 0xff, 0xff, 0xff });
		m_pCircleEnemy = new CCircleSDL(m_appSDL, m_radiusEnemy, { 0xff, 0x00, 0x00, 0xff });
		m_pCircleBullet = new CCircleSDL(m_appSDL, m_radiusBullet, { 0xff, 0x00, 0xff, 0xff });
	}

	void initObjects()
	{
		int screenW, screenH;
		SDL_GetWindowSize(m_appSDL.getWindow(), &screenW, &screenH);

		// init player
		CCircleDanmaku* p = new CCircleDanmaku(m_appSDL, m_pCirclePlayer->getTexture());
		p->setRadius(m_radiusPlayer);
		p->moveTo(static_cast<float>(screenW) / 2.f, static_cast<float>(screenH) / 2.f);
		p->m_health = 3;
		p->m_id = 0;
		p->m_type = PLAYER;
		pushObject(p, PLAYER);

		// init enemy
		srand((unsigned int)time(NULL));
		for (int i = 0; i < m_maxEnemy; i++)
		{
			CCircleDanmaku* p = new CCircleDanmaku(m_appSDL, m_pCircleEnemy->getTexture());
			p->setRadius(m_radiusEnemy);

			float v = 0.f, v_theta = 0.f, x, y;
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
				v = static_cast<float>(rand() * (m_moveSpeed / RAND_MAX));
				v_theta = static_cast<float>(rand() * (2.f * M_PI / RAND_MAX));
			}

			//SDL_Log("Init object %d, (%f, %f) v=%f v_theta=%f", i + 1, x, y, v, v_theta * 180.f / M_PI);
			p->moveTo(x, y);
			p->m_vx = v * cos(v_theta);
			p->m_vy = v * sin(v_theta);
			p->m_theta = v_theta;
			p->m_omiga = static_cast<float>(M_PI / 2);
			p->m_health = 1;
			p->m_id = i + 1;
			p->m_type = ENEMY;
			pushObject(p, ENEMY);
		}
	}

	void releaseObjects()
	{
		for (auto it = m_pObjectMap2DSDL.begin(); it != m_pObjectMap2DSDL.end(); it++)
		{
			for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
			{
				delete *it2;
			}
			(*it).second.clear();
		}
		m_pObjectMap2DSDL.clear();
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
			p1->move(0.f, fmaxf(p1->m_r, p2->m_r));
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
		v1cx2 = ((p1->m_m - p2->m_m) * v1cx + 2 * p2->m_m * v2cx) / (p1->m_m + p2->m_m);
		v2cx2 = ((p2->m_m - p1->m_m) * v2cx + 2 * p1->m_m * v1cx) / (p1->m_m + p2->m_m);
		p1->m_vx = (v1cx2 * cx + v1cy * cy);
		p1->m_vy = (v1cx2 * cy - v1cy * cx);
		p2->m_vx = (v2cx2 * cx + v2cy * cy);
		p2->m_vy = (v2cx2 * cy - v2cy * cx);
		return true;
	}

	// fire a bullet from the object p
	void fireBullet(CCircleDanmaku *p) 
	{
		auto pb = new CCircleDanmaku(m_appSDL, m_pCircleBullet->getTexture());
		pb->setRadius(m_radiusBullet);
		pb->m_health = 1;
		pb->m_vx = m_bulletSpeed * cos(p->m_theta);
		pb->m_vy = m_bulletSpeed * sin(p->m_theta);
		pb->m_x = p->m_x + (p->m_r + 1.1f * pb->m_r) * cos(p->m_theta);
		pb->m_y = p->m_y + (p->m_r + 1.1f * pb->m_r) * sin(p->m_theta);
		pb->m_id = m_pObjectMap2DSDL[BULLENT].size() + 1;
		pb->m_type = BULLENT;
		pushObject(pb, BULLENT);
	}

	void handleEvent(SDL_Event& event)
	{
		if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) return;
		SDL_Scancode scancode = event.key.keysym.scancode;
		
		// game restart
		if (scancode == SDL_SCANCODE_R) 
		{
			releaseObjects();
			initObjects();
			return;
		}		
		
		// check the player
		if (m_pObjectMap2DSDL[PLAYER].size() <= 0) return;
		auto pPlayer = static_cast<CCircleDanmaku*>(*m_pObjectMap2DSDL[PLAYER].begin());
		if (!pPlayer)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CDanmakuStage::handleevent, pPlayer is NULL! ");
			return;
		}
	
		// player control
		int dx = 0, dy = 0;
		float vx = pPlayer->m_vx, vy = pPlayer->m_vy, dtheta = 0.f;
		if (event.type == SDL_KEYDOWN)
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
		    // fire a bullet
			if (scancode == SDL_SCANCODE_SPACE)
			{
				fireBullet(pPlayer);
			}
		}
		pPlayer->m_vx = vx;
		pPlayer->m_vy = vy;
		pPlayer->rotate(dtheta);
	}

	void updateEnemys(Uint32 interval)
	{
		for (auto it = m_pObjectMap2DSDL[ENEMY].begin();
			it != m_pObjectMap2DSDL[ENEMY].end(); it++)
		{
			auto p1 = static_cast<CCircleDanmaku*>(*it);
			if (p1->m_health <= 0) continue;

			p1->move(interval);
			for (auto it2 = m_pObjectMap2DSDL[ENEMY].begin(); it2 != it; it2++)
			{
				auto p2 = static_cast<CCircleDanmaku*>(*it2);
				if (p2->m_health <= 0) continue;
				doCollision(p1, p2);
			}
			for (auto it2 = m_pObjectMap2DSDL[PLAYER].begin(); it2 != m_pObjectMap2DSDL[PLAYER].end(); it2++)
			{
				auto p2 = static_cast<CCircleDanmaku*>(*it2);
				if (p2->m_health <= 0) continue;
				if (doCollision(p1, p2)) 
				{
					p2->m_health--;
				}
			}
		}
	}

	void updatePlayers(Uint32 interval)
	{
		for (auto it = m_pObjectMap2DSDL[PLAYER].begin();
			it != m_pObjectMap2DSDL[PLAYER].end(); it++)
		{
			auto p1 = static_cast<CCircleDanmaku*>(*it);
			p1->move(interval);
			for (auto it2 = m_pObjectMap2DSDL[PLAYER].begin(); it2 != it; it2++)
			{
				auto p2 = static_cast<CCircleDanmaku*>(*it2);
				doCollision(p1, p2);
			}
		}
	}

	void updateBullets(Uint32 interval)
	{
		int screenW, screenH;
		SDL_GetWindowSize(m_appSDL.getWindow(), &screenW, &screenH);
		for (auto it = m_pObjectMap2DSDL[BULLENT].begin();
			it != m_pObjectMap2DSDL[BULLENT].end(); it++)
		{
			auto p1 = static_cast<CCircleDanmaku*>(*it);
			if (p1->m_health <= 0) continue;
			p1->move(interval, false);
			if (p1->m_x<0 || p1->m_x>screenW || p1->m_y<0 || p1->m_vy>screenH)
			{
				p1->m_health = 0;
				continue;
			}
			for (auto it2 = m_pObjectMap2DSDL[ENEMY].begin(); it2 != m_pObjectMap2DSDL[ENEMY].end(); it2++)
			{
				auto p2 = static_cast<CCircleDanmaku*>(*it2);
				if (p2->m_health <= 0) continue;
				if (checkCollision(p1, p2))
				{
					p1->m_health = 0;
					p2->m_health--;
				}
			}
			for (auto it2 = m_pObjectMap2DSDL[PLAYER].begin(); it2 != m_pObjectMap2DSDL[PLAYER].end(); it2++)
			{
				auto p2 = static_cast<CCircleDanmaku*>(*it2);
				if (p2->m_health <= 0) continue;
				if (checkCollision(p1, p2))
				{
					p1->m_health = 0;
					p2->m_health--;
				}
			}
		}
	}

	void removeDead(int type)
	{
		for (auto it = m_pObjectMap2DSDL[type].begin();it != m_pObjectMap2DSDL[type].end();)
		{
			auto p = static_cast<CCircleDanmaku*>(*it);
			if (p->m_health < 0)
			{
				delete removeObject(it, type);
			}
			else
			{
				it++;
			}
		}
	}

	void update()
	{
		Uint32 interval = SDL_GetTicks() - m_lastUpate;
		// update position and velocity by collision
		updateEnemys(interval);
		updatePlayers(interval);
		updateBullets(interval);
		removeDead(BULLENT);
		removeDead(ENEMY);
		removeDead(PLAYER);
		m_lastUpate = SDL_GetTicks();
	}

	~CDanmakuStage()
	{
		releaseObjects();
		delete m_pCirclePlayer, m_pCircleEnemy, m_pCircleBullet;
	}
};

int main(int argc, char* argv[])
{
	CAppSDL app;
	app.prepareWindow("circle danmaku", 800, 600);
	app.prepareGL();
	CStageManegerSDL manager(app);
	CDanmakuStage stage(app);
	manager.pushStage((CStageSDL*)&stage);
	stage.initObjects();
	app.prepareStageManager(&manager);
	app.setBackground(0xff, 0xc0, 0xcb);
	app.setFps(144);
	app.run();
	return 0;
}