/*
simple wrapper of the SDL 2D object
  v0.1, developed by devseed
*/

#ifndef _SDL_OBJECT2D_H
#define _SDL_OBJECT2D_H
#include <memory>
#include "sdl_framework.hpp"
#include "physics_object.hpp"
using std::shared_ptr;
class CAppSDL;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class CObject2DSDL
{
protected:
	CAppSDL& m_appSDL;
public:
	int m_type=0, m_id=0, m_status=0;
	// physical information pointer, can be iterator or object pointer
	// also can inherit physical class other than use related pointer
	shared_ptr<void*> m_pPhysicsRelated = nullptr; 
public:
	CObject2DSDL(CAppSDL& appSDL);
	void setAppSDL(CAppSDL& appSDL);
	CAppSDL& getAppSDL();

	virtual ~CObject2DSDL();
	virtual void draw() = 0;
	virtual void moveTo(float x, float y) = 0;
};

class CSingleTextureSDL : public CObject2DSDL
{
protected: 
	shared_ptr<SDL_Texture> m_texture = nullptr;
public:
	SDL_Rect m_renderRect = {0,0,0,0};
	float m_angle = 0.f;
	shared_ptr<SDL_Rect> m_pSrcRect = nullptr;
	shared_ptr<SDL_Point> m_pCenter = nullptr;
	SDL_RendererFlip flip = SDL_FLIP_NONE;
public:
	static shared_ptr<SDL_Texture> makeTexturePtr(SDL_Texture* texture);
public:
	CSingleTextureSDL(CAppSDL& appSDL);
	CSingleTextureSDL(CAppSDL& appSDL, shared_ptr<SDL_Texture> texture);
	void setTexture(shared_ptr<SDL_Texture> texture);
	shared_ptr<SDL_Texture> getTexture();

	virtual ~CSingleTextureSDL();
	
	// the moving position should be float,for accumulate situations
	// x can be center point or left point
	virtual void moveTo(float center_x, float center_y); 
	virtual void rotateTo(float theta);
	virtual void scaleTo(int w, int h);
	virtual void draw();
};

// simple circle in SDL, for static drawing
class CCircleSDL: public CSingleTextureSDL
{
protected:
	SDL_Color m_color;
	float m_r;
public:
	CCircleSDL(CAppSDL& appSDL);
	CCircleSDL(CAppSDL& appSDL, float radius, SDL_Color color);
	virtual ~CCircleSDL();

	void create(float radius, SDL_Color color,
		Uint32 format= SDL_PIXELFORMAT_RGBA8888, 
		Uint32 access=SDL_TEXTUREACCESS_STREAMING);
	void moveTo(float center_x, float center_y);
	void draw();
};
#endif