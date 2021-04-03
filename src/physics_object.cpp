#include <math.h>
#include<iostream>
#include "physics_object.hpp"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*CPhysicsObject2D start*/
void CPhysicsObject2D::setScreen(int screenW, int screenH)
{
	m_screenW = screenW; 
	m_screenH = screenH;
}

void CPhysicsObject2D::loopScreen(float* x, float* y)
{
	if (m_screenH == 0 || m_screenW == 0) return;
	while (*x > static_cast<float>(m_screenW))
	{
		*x -= static_cast<float>(m_screenW);
	}
	while (*x < 0.f)
	{
		*x += static_cast<float>(m_screenW);
	}
	while (*y > static_cast<float>(m_screenH))
	{
		*y -= static_cast<float>(m_screenH);
	}
	while (*y < 0.f)
	{
		*y += static_cast<float>(m_screenH);
	}
}

void CPhysicsObject2D::loopAngular(float* theta)
{
	while (*theta > static_cast<float>(M_PI))
	{
		*theta -= 2 * static_cast<float>(M_PI);
	}
	while (*theta < static_cast<float>(-M_PI))
	{
		*theta += 2 * static_cast<float>(M_PI);
	}
}

/*CPhysicsObject2D end*/

/*CPhysicsRagid2D start*/
void CPhysicsRagid2D::predict_move(float t, float* vx, float* vy, float* x, float* y, bool loop)
{
	*vx = m_vx + m_ax * t;
	*vy = m_vy + m_ay * t;
	*x = m_x + m_vx * t;
	*y = m_y + m_vy * t;
	if (loop) loopScreen(x, y);
}

void CPhysicsRagid2D::predict_rotate(float t, float* omiga, float* theta, bool loop)
{
	*omiga = m_omiga + m_alpha * t;
	*theta = m_theta + m_omiga * t;
	if (loop) loopAngular(theta);
}

void CPhysicsRagid2D::move(float t, bool loop)
{
	predict_move(t, &m_vx, &m_vy, &m_x, &m_y, loop);
}

void CPhysicsRagid2D::move(float dx, float dy, bool loop)
{
	moveTo(m_x + dx, m_x + dy, loop);
}

void CPhysicsRagid2D::rotate(float t, bool loop)
{
	predict_rotate(t, &m_omiga, &m_theta, loop);
}

void CPhysicsRagid2D::moveTo(float x, float y, bool loop)
{
	m_x = x; m_y = y;
	if (loop) loopScreen(&m_x, &m_y);
}

void CPhysicsRagid2D::rotateTo(float theta, bool loop)
{
	m_theta = theta;
	if (loop) loopAngular(&m_theta);
}
/*CPhysicsRagid2D end*/

/*CPhysicsRagidCircle start*/
bool CPhysicsRagidCircle::checkCollision(CPhysicsRagidCircle* p1, CPhysicsRagidCircle* p2)
{
	return (p2->m_x - p1->m_x) * (p2->m_x - p1->m_x) +
		(p2->m_y - p1->m_y) * (p2->m_y - p1->m_y) <=
		(p1->m_r + p2->m_r) * (p1->m_r + p2->m_r);
}

bool CPhysicsRagidCircle::doCollision(CPhysicsRagidCircle* p1, CPhysicsRagidCircle* p2)
{
	// check if collision
	float cx, cy; // vector c , p1 center -> p2 center vector
	float lc, dcenter; // length of the (cx, cy) vector
	cx = p2->m_x - p1->m_x;
	cy = p2->m_y - p1->m_y;
	lc = sqrtf(cx * cx + cy * cy);
	dcenter = p1->m_r + p2->m_r;
	if (lc > dcenter) return false;
	if (lc == 0.f)
	{
		p1->move(0.f, fmaxf(p1->m_r, p2->m_r));
		return true;
	}
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
	cth = atan2f(cy, cx);
	v1th = atan2f(p1->m_vy, p1->m_vx);
	v2th = atan2f(p2->m_vy, p2->m_vx);
	t = sqrtf(p1->m_vx * p1->m_vx + p1->m_vy * p1->m_vy);
	v1cx = t * cosf(cth - v1th); // the screen coordinate is inverse than the normal coordinate
	v1cy = t * sinf(cth - v1th);
	t = sqrtf(p2->m_vx * p2->m_vx + p2->m_vy * p2->m_vy);
	v2cx = t * cosf(cth - v2th);
	v2cy = t * sinf(cth - v2th);

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

bool CPhysicsRagidCircle::checkCollision(CPhysicsRagidCircle* object)
{
	return checkCollision(this, object);
}

bool CPhysicsRagidCircle::doCollision(CPhysicsRagidCircle* object)
{
	return doCollision(this, object);
}
/*CPhysicsRagidCircle end*/