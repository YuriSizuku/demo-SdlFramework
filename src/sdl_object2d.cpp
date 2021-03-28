#include "sdl_object2d.hpp"
/*CObject2DSDL start*/
CObject2DSDL::CObject2DSDL(CAppSDL& appSDL) : m_appSDL(appSDL) {}

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

/*CCircleSDL start*/
CCircleSDL::CCircleSDL(CAppSDL& appSDL):CObject2DSDL(appSDL)
{
	m_texture = NULL;
}

CCircleSDL::CCircleSDL(CAppSDL& appSDL, int radius, SDL_Color color):CObject2DSDL(appSDL)
{
	m_texture = NULL;
	create(radius, color);
}

CCircleSDL::~CCircleSDL()
{
	releaseTexture();
}

void CCircleSDL::releaseTexture()
{
	if (m_texture != NULL)
	{
		SDL_DestroyTexture(m_texture);
	}
	m_texture = NULL;
}

void CCircleSDL::create(int radius, SDL_Color color, Uint32 format, Uint32 access)
{
	releaseTexture();
	m_renderRect = { 0,0,2 * radius, 2 * radius };
	m_texture = SDL_CreateTexture(m_appSDL.getRenderer(), format, access, 2 * radius, 2 * radius);
	if (!m_texture)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, 
			"In CCircleSDL::create, SDL_CreateTexture failed, %s", SDL_GetError());
	}
	SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
	m_color = color;
	m_radius = radius;

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
				buf[x * 4 + y * 2 * radius * 4] = m_color.r;
				buf[x * 4 + y * 2 * radius * 4 + 1] = m_color.g;
				buf[x * 4 + y * 2 * radius * 4 + 2] = m_color.b;
				buf[x * 4 + y * 2 * radius * 4 + 3] = m_color.a;
			}
		}
	}
	
	SDL_UpdateTexture(m_texture, &m_renderRect, buf, 4 * 2 * radius);
	delete[] buf;
}

void CCircleSDL::moveTo(int center_x, int center_y)
{
	m_renderRect.x = center_x - m_radius;
	m_renderRect.y = center_y - m_radius;
}

void CCircleSDL::draw()
{
	SDL_RenderCopy(getAppSDL().getRenderer(), m_texture, NULL, &m_renderRect);
}
/*CCircleSDL end*/