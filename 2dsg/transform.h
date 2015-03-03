#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Matrices.h"

class Transform
{
public:
	Transform()
	{
		scaleX_=1;
		scaleY_=1;
		scaleZ_=1;
		tx_=0;
		ty_=0;
		tz_=0;
		rotationX_=0;
		rotationY_=0;
		rotationZ_=0;
	}

	void setRotationZ(float r)
	{
		rotationZ_ = r;
		compose();
	}

	void setRotationY(float r)
	{
		rotationY_ = r;
		compose();
	}

	void setRotationX(float r)
	{
		rotationX_ = r;
		compose();
	}

	void setScaleX(float sx)
	{
		scaleX_ = sx;
		compose();
	}

	void setScaleY(float sy)
	{
		scaleY_ = sy;
		compose();
	}

	void setScaleZ(float sz)
	{
		scaleZ_ = sz;
		compose();
	}

	void setScaleXY(float sx, float sy)
	{
		scaleX_ = sx;
		scaleY_ = sy;
		compose();
	}

	void setScaleXYZ(float sx, float sy,float sz)
	{
		scaleX_ = sx;
		scaleY_ = sy;
		scaleZ_ = sz;
		compose();
	}

	void setX(float x)
	{
		tx_=x;
		compose();
	}

	void setY(float y)
	{
		ty_=y;
		compose();
	}

	void setZ(float z)
	{
		tz_=z;
		compose();
	}

	void setXY(float x, float y)
	{
		tx_=x;
		ty_=y;
		compose();
	}

	void setXYZ(float x, float y, float z)
	{
		tx_=x;
		ty_=y;
		tz_=z;
		compose();
	}

	float rotationX() const
	{
		return rotationX_;
	}

	float rotationY() const
	{
		return rotationY_;
	}

	float rotationZ() const
	{
		return rotationZ_;
	}

	float scaleX() const
	{
		return scaleX_;
	}

	float scaleY() const
	{
		return scaleY_;
	}

	float scaleZ() const
	{
		return scaleZ_;
	}

	float x() const
	{
		return tx_;
	}

	float y() const
	{
		return ty_;
	}

	float z() const
	{
		return tz_;
	}

	const Matrix4& matrix() const
	{
		return matrix_;
	}
	void setMatrix(float m11,float m12,float m21,float m22,float tx,float ty);
private:
	Matrix4 matrix_;

	float rotationX_;
	float rotationY_;
	float rotationZ_;
	float scaleX_;
	float scaleY_;
	float scaleZ_;
	float tx_,ty_,tz_;
	mutable bool isDirty_;
	void compose();
};


class TransformContainer
{
public:
	TransformContainer(Transform* transform) : transform(transform)
	{
	}

	TransformContainer(const Transform& t) : transformInstance(t)
	{
		transform = &transformInstance;
	}

	Transform* transform;
	Transform transformInstance;
};



#endif
