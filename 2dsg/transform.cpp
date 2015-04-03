#include "transform.h"
#include <glog.h>

#ifndef M_PI
#define M_PI 3.141592654
#endif

static void multiply(const float m0[2][2], const float m1[2][2], float result[2][2])
{
 float r[2][2];

 r[0][0] = m0[0][0] * m1[0][0] + m0[0][1] * m1[1][0];
 r[0][1] = m0[0][0] * m1[0][1] + m0[0][1] * m1[1][1];
 r[1][0] = m0[1][0] * m1[0][0] + m0[1][1] * m1[1][0];
 r[1][1] = m0[1][0] * m1[0][1] + m0[1][1] * m1[1][1];

 result[0][0] = r[0][0];
 result[0][1] = r[0][1];
 result[1][0] = r[1][0];
 result[1][1] = r[1][1];
}


static void rotation(float angle, float result[2][2])
{
 result[0][0] = std::cos(angle);
 result[0][1] = -std::sin(angle);
 result[1][0] = std::sin(angle);
 result[1][1] = std::cos(angle);
}

void Transform::setMatrix(float m11,float m12,float m21,float m22,float tx,float ty)
{
   float m[2][2] = {{m11, m12}, {m21, m22}};

	bool bx = m[0][0] == 0 && m[1][0] == 0;
	bool by = m[0][1] == 0 && m[1][1] == 0;

	float sx = std::sqrt(m[0][0] * m[0][0] + m[1][0] * m[1][0]);
	float sy = std::sqrt(m[0][1] * m[0][1] + m[1][1] * m[1][1]);
	float angle = std::atan2(m[1][0], m[0][0]);

	float r[2][2];
	::rotation(-angle, r);
	multiply(r, m, m);

	if (m[1][1] < 0)
		sy = -sy;

	rotationZ_ = angle * 180 / M_PI;
	rotationX_=0;
	rotationY_=0;
	scaleX_ = bx ? 0 : sx;
	scaleY_ = by ? 0 : sy;
	scaleZ_=0;

	tx_=tx;
	ty_=ty;
	tz_=0;
}

void Transform::compose()
{
#if 1
	matrix_.identity();
	matrix_.translate(-refX_,-refY_,-refZ_);
	matrix_.scale(scaleX_,scaleY_,scaleZ_);
	if (rotationZ_)
		matrix_.rotateZ(rotationZ_);
	if (rotationY_)
		matrix_.rotateY(rotationY_);
	if (rotationX_)
		matrix_.rotateX(rotationX_);
	matrix_.translate(tx_,ty_,tz_);
	isDirty_=false;

#else
	// optimized version of the code above
	// {{cos(a),-sin(a)},{sin(a),cos(a)}} * {{x, u * y},{0,v * y}}
	float c = cos(rotation_ * M_PI / 180);
	float s = sin(rotation_ * M_PI / 180);


#endif

}