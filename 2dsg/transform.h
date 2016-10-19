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
        skewX_=0;
        skewY_=0;
		tx_=0;
		ty_=0;
		tz_=0;
		rotationX_=0;
		rotationY_=0;
		rotationZ_=0;
        refX_ = 0;
        refY_ = 0;
        refZ_ = 0;
        isDirty_=false;
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

    void setSkewX(float sx)
    {
        skewX_ = sx;
        compose();
    }

    void setSkewY(float sy)
    {
        skewY_ = sy;
        compose();
    }

    void setSkewXY(float sx, float sy)
    {
        skewX_ = sx;
        skewY_ = sy;
        compose();
    }

	void setX(float x)
	{
		matrix_[12]+=(x-tx_);
		tx_=x;
	}

	void setY(float y)
	{
		matrix_[13]+=(y-ty_);
		ty_=y;
	}

	void setZ(float z)
	{
		matrix_[14]+=(z-tz_);
		tz_=z;
	}

	void setXY(float x, float y)
	{
		matrix_[12]+=(x-tx_);
		matrix_[13]+=(y-ty_);
		tx_=x;
		ty_=y;
	}

	void setXYZ(float x, float y, float z)
	{
		matrix_[12]+=(x-tx_);
		matrix_[13]+=(y-ty_);
		matrix_[14]+=(z-tz_);
		tx_=x;
		ty_=y;
		tz_=z;
	}

    void setRefX(float x)
    {
        refX_ = x;
		compose();
    }

    void setRefY(float y)
    {
        refY_ = y;
		compose();
    }

    void setRefZ(float z)
    {
        refZ_ = z;
		compose();
    }

    void setRefXY(float x, float y)
    {
        refX_ = x;
        refY_ = y;
		compose();
    }

    void setRefXYZ(float x, float y, float z)
    {
        refX_ = x;
        refY_ = y;
        refZ_ = z;
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

    float skewX() const
    {
        return skewX_;
    }

    float skewY() const
    {
        return skewY_;
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

    float refX() const
    {
        return refX_;
    }

    float refY() const
    {
        return refY_;
    }

    float refZ() const
    {
        return refZ_;
    }

	const Matrix4& matrix() const
	{
		return matrix_;
	}
	void setMatrix(float m11,float m12,float m21,float m22,float tx,float ty);
	void setMatrix(const float *m);
private:
	Matrix4 matrix_;

	float rotationX_;
	float rotationY_;
	float rotationZ_;
	float scaleX_;
	float scaleY_;
	float scaleZ_;
    float skewX_;
    float skewY_;
	float tx_,ty_,tz_;
    float refX_;
    float refY_;
    float refZ_;
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
