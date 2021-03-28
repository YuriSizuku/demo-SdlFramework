#ifndef _SDL_OBJECT2D_H
#define _SDL_OBJECT2D_H
#include "sdl_framework.hpp"
class CObject2DSDL
{
protected:
	CAppSDL& m_appSDL;
	SDL_Rect m_renderRect;
public:
	CObject2DSDL(CAppSDL& appSDL);
	void setAppSDL(CAppSDL& appSDL);
	CAppSDL& getAppSDL();
	virtual ~CObject2DSDL();
	virtual void moveTo(int x, int y) = 0;
	virtual void draw() = 0;
};

class CCircleSDL: public CObject2DSDL
{
protected:
	SDL_Texture* m_texture;
	SDL_Color m_color;
	int m_radius;
public:
	CCircleSDL(CAppSDL& appSDL);
	CCircleSDL(CAppSDL& appSDL, int radius, SDL_Color color);
	virtual ~CCircleSDL();
	void releaseTexture();
	
	void create(int radius, SDL_Color color,
		Uint32 format= SDL_PIXELFORMAT_RGBA8888, Uint32 access=SDL_TEXTUREACCESS_STREAMING);
	void moveTo(int center_x, int center_y);
	void draw();
};

#endif