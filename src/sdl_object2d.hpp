/*
  simple wrapper of the SDL 2D object
  by devseed
  v0.1
*/

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
	// the moving position should be float,for accumulate situations
	virtual void moveTo(float x, float y) = 0; // x can be center point or left point
	virtual void draw() = 0;
};

// ragid body object
class CRagidSDL: public CObject2DSDL
{
public:
	float m_x, m_y, m_m; // the object position, mass
	float m_vx, m_vy; // the object volocity, pixels in a second
	float m_ax, m_ay; // the object accelerate volocity
	float m_theta, m_omiga; // m_theta object angle, m_omiga angular speed
public:
	CRagidSDL(CAppSDL& appSDL);
	virtual ~CRagidSDL();
	virtual void predict_move(Uint32 interval, float* vx, float* vy, 
		float* x, float* y, bool loopScreen=true);
	virtual void move(Uint32 interval, bool loopScreen=true);
	virtual void move(float dx, float dy, bool loopScreen=true);
	virtual void rotate(float dtheta);
	virtual void rotate(Uint32 interval);
	virtual void moveTo(float x, float y);
	virtual void rotateTo(float theta);
};

class CCircleSDL: public CObject2DSDL
{
protected:
	SDL_Texture* m_texture;
	SDL_Color m_color;
	float m_radius;
public:
	CCircleSDL(CAppSDL& appSDL);
	CCircleSDL(CAppSDL& appSDL, float radius, SDL_Color color);
	virtual ~CCircleSDL();
	SDL_Texture *getTexture();
	void releaseTexture();

	void create(float radius, SDL_Color color,
		Uint32 format= SDL_PIXELFORMAT_RGBA8888, 
		Uint32 access=SDL_TEXTUREACCESS_STREAMING);
	void moveTo(float center_x, float center_y);
	void draw();
};

#endif