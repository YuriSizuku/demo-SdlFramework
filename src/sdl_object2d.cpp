#include <string.h>
#include "sdl_object2d.hpp"
/*CObject2DSDL start*/
CObject2DSDL::CObject2DSDL(CAppSDL& appSDL) : m_appSDL(appSDL) 
{
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

/*CSceneSDL start*/
CSceneSDL::CSceneSDL(CAppSDL& appSDL):m_appSDL(appSDL)
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

/*CSingleTextureSDL start*/

shared_ptr<SDL_Texture> CSingleTextureSDL::makeTexturePtr(SDL_Texture* texture)
{
	return shared_ptr<SDL_Texture>(texture, [](SDL_Texture* texture) {SDL_DestroyTexture(texture); });
}

CSingleTextureSDL::CSingleTextureSDL(CAppSDL& appSDL):CObject2DSDL(appSDL)
{

}

CSingleTextureSDL::CSingleTextureSDL(CAppSDL& appSDL, shared_ptr<SDL_Texture> texture) : CObject2DSDL(appSDL)
{
	setTexture(texture);
}

CSingleTextureSDL::~CSingleTextureSDL() {}

shared_ptr<SDL_Texture> CSingleTextureSDL::getTexture()
{
	return m_texture;
}

void CSingleTextureSDL::setTexture(shared_ptr<SDL_Texture> texture)
{
	Uint32 format;
	int access, w, h;
	m_texture = texture;
	SDL_QueryTexture(m_texture.get(), &format, &access, &w, &h);
}

void CSingleTextureSDL::moveTo(float center_x, float center_y)
{
	m_renderRect.x = static_cast<int>(round(center_x 
		- static_cast<float>(m_renderRect.w)/2.f));
	m_renderRect.y = static_cast<int>(round(center_y 
		- static_cast<float>(m_renderRect.h)/2.f));
}

void CSingleTextureSDL::rotateTo(float theta)
{
	m_angle = theta * 180 / static_cast<float>(M_PI);
}

void CSingleTextureSDL::scaleTo(int w, int h)
{
	m_renderRect.w = w;
	m_renderRect.h = h;
}

void CSingleTextureSDL::draw()
{
	if (m_texture == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CSingleTextureSDL::draw() texture is NULL");
	}
	SDL_RenderCopyEx(m_appSDL.getRenderer(), m_texture.get(),
		m_pSrcRect.get(), &m_renderRect, m_angle, m_pCenter.get(), flip);
}
/*CSingleTextureSDL end*/

CCircleSDL::CCircleSDL(CAppSDL& appSDL):CSingleTextureSDL(appSDL)
{
	m_color = { 0,0,0,0 };
	m_r = 0;
	m_texture = NULL;
}

CCircleSDL::CCircleSDL(CAppSDL& appSDL, float radius, SDL_Color color):CSingleTextureSDL(appSDL)
{
	m_texture = NULL;
	create(radius, color);
}

CCircleSDL::~CCircleSDL()
{

}

void CCircleSDL::create(float radiusf, SDL_Color color, Uint32 format, Uint32 access)
{
	int radius = static_cast<int>(round(radiusf));
	if (!radius)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "CCircleSDL::create failed with radius 0");
		return;
	}
	m_renderRect = { 0,0,2 * radius, 2 * radius };
	SDL_Texture* texture = SDL_CreateTexture(m_appSDL.getRenderer(), format, access, 2 * radius, 2 * radius);
	m_texture = makeTexturePtr(texture);
	if (!m_texture)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ASSERT, 
			"In CCircleSDL::create, SDL_CreateTexture failed, %s", SDL_GetError());
	}
	SDL_SetTextureBlendMode(m_texture.get(), SDL_BLENDMODE_BLEND);
	m_color = color;
	m_r = radiusf;

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
	
	SDL_UpdateTexture(m_texture.get(), &m_renderRect, buf, 4 * 2 * radius);
	delete[] buf;
}

void CCircleSDL::moveTo(float center_x, float center_y)
{
	m_renderRect.x = static_cast<int>(round(center_x - m_r));
	m_renderRect.y = static_cast<int>(round(center_y - m_r));
}

void CCircleSDL::draw()
{
	SDL_RenderCopy(getAppSDL().getRenderer(), m_texture.get(), NULL, &m_renderRect);
}
/*CCircleSDL end*/