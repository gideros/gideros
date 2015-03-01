#include "transform.h"


void Transform::decompose()
{
/* TODO    float m[2][2] = {{matrix_.m11(), matrix_.m12()}, {matrix_.m21(), matrix_.m22()}};

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
*/
}

void Transform::compose()
{
	matrix_.identity();
	matrix_.scale(scaleX_,scaleY_,scaleZ_);
	if (rotationZ_)
		matrix_.rotateZ(rotationZ_);
	if (rotationY_)
		matrix_.rotateY(rotationY_);
	if (rotationX_)
		matrix_.rotateX(rotationX_);
	matrix_.translate(tx_,ty_,tz_);
	isDirty_=false;
}
