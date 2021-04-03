/*
* simple physics engine framework for 2D or 3D object
* developed by devseed
* v0.1
*/
#include <math.h>
#include "data_types.hpp"
#ifndef _PHYSICS_OBJECT_H
#define _PHYSICS_OBJECT_H
template <typename T>
class IPhysicsCollision
{
public:
	// check if object1 crash object2
	virtual bool checkCollision(T* object) = 0;
	// collision simulation, object1 crash object2
	virtual bool doCollision(T* object) = 0;
};

class CPhysicsObject
{
public:
	float m_m=1.f;// object mass
};

class CPhysicsObject2D: public CPhysicsObject
{
protected:
	int m_screenH = 0, m_screenW = 0;
public:
	void setScreen(int screenW, int screenH);
	void loopScreen(float* x, float* y);
	void loopAngular(float* theta);
};

class CPhysicsRagid2D: public CPhysicsObject2D
{
public:
	float m_x=0.f, m_y=0.f; // the object position, mass
	float m_vx=0.f, m_vy=0.f; // the object volocity, pixels in a second
	float m_ax=0.f, m_ay=0.f; // the object accelerate volocity
	float m_theta=0.f, m_omiga=0.f, m_alpha=0.f; // m_theta object angle, m_omiga angular speed

	virtual void predict_move(float t, float* vx, float* vy, float* x, float* y, bool loop=true);
	virtual void predict_rotate(float t, float* m_omiga, float* theta, bool loop=true);
	virtual void move(float t, bool loop=true);
	virtual void move(float dx, float dy, bool loop=true);
	virtual void rotate(float t, bool loop = true);
	virtual void moveTo(float x, float y, bool loop=true);
	virtual void rotateTo(float theta, bool loop=true);
};

class CPhysicsRagidCircle : public CPhysicsRagid2D, public IPhysicsCollision<CPhysicsRagidCircle>
{
public:
	float m_r = 1.f; // the circle radius
public:
	static bool checkCollision(CPhysicsRagidCircle* p1, CPhysicsRagidCircle* p2);
	static bool doCollision(CPhysicsRagidCircle* p1, CPhysicsRagidCircle* p2);
public:
	virtual bool checkCollision(CPhysicsRagidCircle* object);
	virtual bool doCollision(CPhysicsRagidCircle* object);
};

class CPhysicsField
{
public:
	CMapList<CPhysicsObject*> pObjects;
	virtual void update() = 0;
};

#endif