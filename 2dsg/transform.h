#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "matrix.h"

class Transform
{
public:
	Transform()
	{
		decompose();
		isDirty_ = true;
        x_ = 0;
        y_ = 0;
        refX_ = 0;
        refY_ = 0;
	}

	void setRotation(float r)
	{
		rotation_ = r;
		compose();
		isDirty_ = true;
	}

	void setScaleX(float sx)
	{
		scaleX_ = sx;
		compose();
		isDirty_ = true;
	}

	void setScaleY(float sy)
	{
		scaleY_ = sy;
		compose();
		isDirty_ = true;
	}

	void setScaleXY(float sx, float sy)
	{
		scaleX_ = sx;
		scaleY_ = sy;
		compose();
		isDirty_ = true;
	}

	void setX(float x)
	{
        matrix_.setTx(x-refX_);
        x_ = x;
		isDirty_ = true;
	}

	void setY(float y)
	{
        matrix_.setTy(y-refY_);
        y_ = y;
		isDirty_ = true;
	}

	void setXY(float x, float y)
	{
        matrix_.setTx(x-refX_);
        matrix_.setTy(y-refY_);
        x_ = x;
        y_ = y;
		isDirty_ = true;
	}

    void setRefX(float x)
    {
        matrix_.setTx(x_-x);
        refX_ = x;
        isDirty_ = true;
    }

    void setRefY(float y)
    {
        matrix_.setTy(y_-y);
        refY_ = y;
        isDirty_ = true;
    }

    void setRefXY(float x, float y)
    {
        matrix_.setTx(x_-x);
        matrix_.setTy(y_-y);
        refX_ = x;
        refY_ = y;
        isDirty_ = true;
    }

	float rotation() const
	{
		return rotation_;
	}

	float scaleX() const
	{
		return scaleX_;
	}

	float scaleY() const
	{
		return scaleY_;
	}

	float x() const
	{
        return x_;
	}

	float y() const
	{
        return y_;
	}

    float refX() const
    {
        return refX_;
    }

    float refY() const
    {
        return refY_;
    }

    void setMatrix(float m11, float m12, float m21, float m22, float tx, float ty)
	{
        matrix_.set(m11, m12, m21, m22, tx, ty);
		decompose();
		isDirty_ = true;
	}

	void setMatrix(const Matrix& matrix)
	{
		matrix_ = matrix;
		decompose();
		isDirty_ = true;
	}

	const Matrix& matrix() const
	{
		return matrix_;
	}

	bool isDirty() const
	{
		return isDirty_;
	}

	void setClean() const
	{
		isDirty_ = false;
	}

private:
	Matrix matrix_;

	float rotation_;
	float scaleX_;
	float scaleY_;
    float x_;
    float y_;
    float refX_;
    float refY_;
	float vx_, vy_;

	void decompose();
	void compose();

	mutable bool isDirty_;
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
