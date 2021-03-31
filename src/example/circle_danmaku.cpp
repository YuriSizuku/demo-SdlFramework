#define GLEW_STATIC
#include<GL/glew.h>
#include <SDL.h>
#include <iostream>
#include "sdl_framework.hpp"

enum CIRCLE_TYPE{
	PLAYER,
	ENEMY,
	BULLENT
};

class CCircleDanmaku :public CObject2DSDL // the basic circle of the obejct
{
public:
	SDL_Texture* m_texture;
	
	int m_screenW, m_screenH;
	float m_r, m_m;
	float m_x, m_y, m_vx, m_vy, m_ax, m_ay; // speed, pixels in 1s,  m_vtheta speed angle
	float m_theta, m_omiga;// , m_theta object angle, m_omiga angular speed
	int m_health;

	int m_id;
	CIRCLE_TYPE m_type;

	CCircleDanmaku(CAppSDL& appSDL, SDL_Texture* texture): CObject2DSDL(appSDL)
	{
		m_texture = texture;
		SDL_GetWindowSize(m_appSDL.getWindow(), &m_screenW, &m_screenH);
		m_x = 0; m_y = 0; m_r = 0; m_health = 0;
		m_m = 1.f; m_vx = 0.f; m_vy = 0.f; m_ax = 0.f; m_ay = 0.f;
		m_theta = 0.f; m_omiga =0.f;
		m_id = 0; m_type = PLAYER;
	}

	void setRadius(float r)
	{
		m_r = r;
		m_renderRect.w = static_cast<int>(m_r * 2);
		m_renderRect.h = static_cast<int>(m_r * 2);
	}

	void predict_move(Uint32 interval, float *vx, float *vy, float *x, float *y)
	{
		*x = m_x;
		*y = m_y;
		*vx = m_vx + m_ax * interval / 1000.f;
		*vy = m_vy + m_ay * interval / 1000.f;
		float dx = m_vx * interval / 1000.f;
		float dy = m_vy * interval / 1000.f;
	
		*x += dx;
		while (*x > static_cast<float>(m_screenW))
		{
			*x -= static_cast<float>(m_screenW);
		}
		while (*x < 0.f)
		{
			*x += static_cast<float>(m_screenW);
		}

		*y += dy;
		while (*y > static_cast<float>(m_screenH))
		{
			*y -= static_cast<float>(m_screenH);
		}
		while (*y < 0.f)
		{
			*y += static_cast<float>(m_screenH);
		}
	}

	void move(Uint32 interval)
	{
		float  x, y, vx, vy;
		predict_move(interval, &vx, &vy, &x, &y);
		m_vx = vx; m_vy = vy;
		moveTo(x, y);
	}

	void rotate(Uint32 interval)
	{
		rotate(static_cast<float>(m_omiga * interval / 1000.f));
	}

	void rotate(float dtheta)
	{
		m_theta += dtheta;
	}

	void moveTo(float x, float y)
	{
		m_x = x;
		m_y = y;
		m_renderRect.x = static_cast<int>(round(x - m_r));
		m_renderRect.y = static_cast<int>(round(y - m_r));
	}

	void rotateTo(float theta)
	{
		m_theta = theta;
	}

	virtual void draw()
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
};

class CDanmakuStage :public CStageSDL // the danmaku game code
{
private:
	Uint32 m_lastUpate;	
	CCircleSDL *m_pCirclePlayer, *m_pCircleEnemy, *m_pCircleBullet;
	CCircleDanmaku* m_pPlayer;
public:
	float m_radiusEnemy, m_radiusPlayer, m_radiusBullet;
	int m_maxEnemy;
	float m_moveStep, m_rotateStep; // move angle in one step 
public:
	CDanmakuStage(CAppSDL& appSDL) :CStageSDL(appSDL)
	{
		// init paramers
		m_lastUpate = SDL_GetTicks();
		m_maxEnemy = 50;
		m_radiusEnemy = 15.f;
		m_radiusPlayer = 15.f;
		m_radiusBullet = 5.f;

		m_moveStep = 100.f; //pixel velocity in a second
		m_rotateStep = static_cast<float>(30 * M_PI / 180);
		
		// init game objects
		initObjects();
	}

	void initObjects()
	{
		m_pCirclePlayer = new CCircleSDL(m_appSDL, m_radiusPlayer, { 0x00, 0xff, 0xff, 0xff });
		m_pCircleEnemy = new CCircleSDL(m_appSDL, m_radiusEnemy, { 0xff, 0x00, 0x00, 0xff });
		m_pCircleBullet = new CCircleSDL(m_appSDL, m_radiusBullet, { 0xff, 0x00, 0xff, 0xff });

		int screen_w, screen_h;
		SDL_GetWindowSize(m_appSDL.getWindow(), &screen_w, &screen_h);
		CCircleDanmaku* p = new CCircleDanmaku(m_appSDL, m_pCirclePlayer->getTexture());
		p->setRadius(m_radiusPlayer);
		p->moveTo(screen_w / 2, screen_h / 2);
		p->m_health = 3;
		m_pPlayer = p;
		p->m_id = 0;
		p->m_type = PLAYER;

		pushObject(p);
		for (int i = 0; i < m_maxEnemy; i++)
		{
			CCircleDanmaku* p = new CCircleDanmaku(m_appSDL, m_pCircleEnemy->getTexture());
			p->setRadius(m_radiusEnemy);
			p->moveTo((i+1)%int(sqrt(m_radiusEnemy)) * screen_w/sqrt(m_radiusEnemy),
				      (i+1)/int(sqrt(m_radiusEnemy)) * screen_h/sqrt(m_radiusEnemy));
			float v = 200.f;
			float v_theta = static_cast<float>((i+1) *(360.f/ m_maxEnemy) * M_PI / 180.f);
			p->m_vx = v*cos(v_theta);
			p->m_vy = v*sin(v_theta);
			p->m_theta = v_theta;
			p->m_omiga = static_cast<float>(M_PI / 2);
			p->m_health = 1;
			p->m_id = i + 1;
			p->m_type = ENEMY;
			pushObject(p);
		}
	}

	void releaseObjects()
	{
		for (auto it = m_pObjects2DSDL.begin(); it != m_pObjects2DSDL.end(); it++)
		{
			delete* it;
		}
		m_pObjects2DSDL.clear();
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
		cx /= lc; cy /= lc; // make normalized

		// make sure tangent point not in the inner cicle
		if (dcenter - lc > 1.f)
		{
			p1->m_x = p2->m_x - 1.01 * dcenter * cx;
			p1->m_y = p2->m_y - 1.01 * dcenter * cy;
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
		v1cx = t * cos(cth - v1th); // the screen coordinate is different than the normal
		v1cy = t * sin(cth - v1th);
		t = sqrtf(p2->m_vx * p2->m_vx + p2->m_vy * p2->m_vy);
		v2cx = t * cos(cth - v2th);
		v2cy = t * sin(cth - v2th);
		
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
				releaseObjects();
				initObjects();
			}
			// player move start
			if (scancode == SDL_SCANCODE_W || scancode == SDL_SCANCODE_UP)
				vy = -m_moveStep;
			if (scancode == SDL_SCANCODE_A || scancode == SDL_SCANCODE_LEFT)
				vx = -m_moveStep;
			if (scancode == SDL_SCANCODE_S || scancode == SDL_SCANCODE_DOWN)
				vy = m_moveStep;
			if (scancode == SDL_SCANCODE_D || scancode == SDL_SCANCODE_RIGHT)
				vx = m_moveStep;
			if (scancode == SDL_SCANCODE_J)
				dtheta = -m_rotateStep;
			if (scancode == SDL_SCANCODE_K)
				dtheta = m_rotateStep;
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

	~CDanmakuStage()
	{
		releaseObjects();
		delete m_pCirclePlayer, m_pCircleEnemy, m_pCircleBullet;
	}
};

int main(int argc, char* argv[])
{
	SDL_Log("This is a simple danmaku demo using sdl framework");

	CAppSDL app;
	app.prepareWindow("circle danmaku", 800, 600);
	app.prepareGL();
	CStageManegerSDL manager(app);
	CDanmakuStage stage(app);
	manager.pushStage((CStageSDL*)&stage);
	app.prepareStageManager(&manager);
	app.setBackground(0xff, 0xc0, 0xcb);
	app.setFps(144);
	app.run();
	return 0;
}