#include "sdl_object2d.hpp"
/*CObject2DSDL start*/
CObject2DSDL::CObject2DSDL(CAppSDL& appSDL) : m_appSDL(appSDL) 
{
	m_renderRect = { 0,0,0,0 };
}

CObject2DSDL::~CObject2DSDL(){}

void CObject2DSDL::setAppSDL(CAppSDL& appSDL)
{
	m_appSDL = appSDL;
}

CAppSDL& CObject2DSDL::getAppSDL()
{
	return m_appSDL;
}
/*CObject2DSDL end*/

/*CRagidSDL start*/
CRagidSDL::CRagidSDL(CAppSDL& appSDL): CObject2DSDL(appSDL)
{
	m_x = 0; m_y = 0; m_m = 1.f;
	m_vx = 0.f; m_vy = 0.f; 
	m_ax = 0.f; m_ay = 0.f;
	m_theta = 0.f; m_omiga = 0.f;
}

CRagidSDL::~CRagidSDL()
{

}

void CRagidSDL::predict_move(Uint32 interval, float* vx, float* vy,
	float* x, float* y, bool loopScreen)
{
	*x = m_x;
	*y = m_y;
	*vx = m_vx + m_ax * interval / 1000.f;
	*vy = m_vy + m_ay * interval / 1000.f;
	float dx = m_vx * interval / 1000.f;
	float dy = m_vy * interval / 1000.f;
	*x += dx;*y += dy;
	if (!loopScreen) return;

	int screenW, screenH;
	SDL_GetWindowSize(m_appSDL.getWindow(), &screenW, &screenH);
	while (*x > static_cast<float>(screenW))
	{
		*x -= static_cast<float>(screenW);
	}
	while (*x < 0.f)
	{
		*x += static_cast<float>(screenW);
	}
	while (*y > static_cast<float>(screenH))
	{
		*y -= static_cast<float>(screenH);
	}
	while (*y < 0.f)
	{
		*y += static_cast<float>(screenH);
	}
}

void CRagidSDL::move(Uint32 interval, bool loop)
{
	float  x, y, vx, vy;
	predict_move(interval, &vx, &vy, &x, &y, loop);
	m_vx = vx; m_vy = vy;
	moveTo(x, y);
}

void CRagidSDL::move(float dx, float dy, bool loop)
{
	m_x += dx; m_y += dy;
	if (!loop) return;

	int screenW, screenH;
	SDL_GetWindowSize(m_appSDL.getWindow(), &screenW, &screenH);
	while (m_x > static_cast<float>(screenW))
	{
		m_x -= static_cast<float>(screenW);
	}
	while (m_x < 0.f)
	{
		m_x += static_cast<float>(screenW);
	}
	while (m_y > static_cast<float>(screenH))
	{
		m_y -= static_cast<float>(screenH);
	}
	while (m_y < 0.f)
	{
		m_y += static_cast<float>(screenH);
	}
}

void CRagidSDL::rotate(Uint32 interval,  bool loop)
{
	float dtheta = m_omiga * static_cast<float>(interval) / 1000.f;
	rotate(dtheta, loop);
}

void CRagidSDL::rotate(float dtheta, bool loop)
{
	rotateTo(m_theta + dtheta, loop);
}

void CRagidSDL::moveTo(float x, float y)
{
	m_x = x; m_y = y;
	m_renderRect.x = static_cast<int>(round(x - m_renderRect.w/2.f));
	m_renderRect.y = static_cast<int>(round(y - m_renderRect.h/2.f));
}

void CRagidSDL::rotateTo(float theta, bool loop)
{
	if (loop)
	{
		while (theta > M_PI) theta -= 2 * M_PI;
		while (theta < -M_PI) theta += 2 * M_PI;
	}
	m_theta = theta;
}
/*CRagidSDL end*/

/*CCircleSDL start*/
CCircleSDL::CCircleSDL(CAppSDL& appSDL):CObject2DSDL(appSDL)
{
	m_color = { 0,0,0,0 };
	m_radius = 0;
	m_texture = NULL;
}

CCircleSDL::CCircleSDL(CAppSDL& appSDL, float radius, SDL_Color color):CObject2DSDL(appSDL)
{
	m_texture = NULL;
	create(radius, color);
}

CCircleSDL::~CCircleSDL()
{
	releaseTexture();
}

SDL_Texture* CCircleSDL::getTexture()
{
	return m_texture;
}

void CCircleSDL::releaseTexture()
{
	if (m_texture != NULL)
	{
		SDL_DestroyTexture(m_texture);
	}
	m_texture = NULL;
}

void CCircleSDL::create(float radiusf, SDL_Color color, Uint32 format, Uint32 access)
{
	releaseTexture();
	int radius = static_cast<int>(round(radiusf));
	if (!radius)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CCircleSDL::create failed with radius 0");
		return;
	}
	m_renderRect = { 0,0,2 * radius, 2 * radius };
	m_texture = SDL_CreateTexture(m_appSDL.getRenderer(), format, access, 2 * radius, 2 * radius);
	if (!m_texture)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, 
			"In CCircleSDL::create, SDL_CreateTexture failed, %s", SDL_GetError());
	}
	SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
	m_color = color;
	m_radius = radiusf;

	Uint8* buf = new Uint8[4 * 2 * radius * 2 * radius];
	for (int x = 0; x < 2 * radius; x++)
	{
		for (int y = 0; y < 2 * radius; y++)
		{
			if ((x - radius) * (x - radius) + (y - radius) * (y - radius) > radius * radius)
			{
				memset(&buf[x * 4 + y * 2 * radius * 4], 0, 4);
			}
			else
			{
				buf[x * 4 + y * 2 * radius * 4] = m_color.a;
				buf[x * 4 + y * 2 * radius * 4 + 1] = m_color.b;
				buf[x * 4 + y * 2 * radius * 4 + 2] = m_color.g;
				buf[x * 4 + y * 2 * radius * 4 + 3] = m_color.r;
			}
		}
	}
	
	SDL_UpdateTexture(m_texture, &m_renderRect, buf, 4 * 2 * radius);
	delete[] buf;
}

void CCircleSDL::moveTo(float center_x, float center_y)
{
	m_renderRect.x = static_cast<int>(round(center_x - m_radius));
	m_renderRect.y = static_cast<int>(round(center_y - m_radius));
}

void CCircleSDL::draw()
{
	SDL_RenderCopy(getAppSDL().getRenderer(), m_texture, NULL, &m_renderRect);
}
/*CCircleSDL end*/