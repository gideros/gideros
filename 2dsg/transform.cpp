#include "transform.h"

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


void Transform::decompose()
{
    float m[2][2] = {{matrix_.m11(), matrix_.m12()}, {matrix_.m21(), matrix_.m22()}};

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

	rotation_ = angle * 180 / M_PI;
	scaleX_ = bx ? 0 : sx;
	scaleY_ = by ? 0 : sy;

	vx_ = m[0][1] / sy;
	vy_ = m[1][1] / sy;

}

void Transform::compose()
{
#if 1
	float m[2][2] = {{scaleX_, vx_ * scaleY_}, {0, vy_ * scaleY_}};

	float r[2][2];
	::rotation(rotation_ * M_PI / 180, r);
	multiply(r, m, m);

    matrix_.setM11(m[0][0]);
    matrix_.setM12(m[0][1]);
    matrix_.setM21(m[1][0]);
    matrix_.setM22(m[1][1]);
#else
	// optimized version of the code above
	// {{cos(a),-sin(a)},{sin(a),cos(a)}} * {{x, u * y},{0,v * y}}
	float c = cos(rotation_ * M_PI / 180);
	float s = sin(rotation_ * M_PI / 180);



#endif




}
