#ifndef MATRIX_H
#define MATRIX_H

#include <cmath>

class Matrix2D
{
public:
	enum Type
	{
		eIdentity,
		eTranslationOnly,
		eArbitrary,
	};

    Matrix2D(float m11 = 1, float m12 = 0, float m21 = 0, float m22 = 1, float tx = 0, float ty = 0)
	{
		for (int i = 0; i < 16; ++i)
			m_[i] = i % 5 ? 0.f : 1.f;

        set(m11, m12, m21, m22, tx, ty);
		setType();
	}

    void set(float m11 = 1, float m12 = 0, float m21 = 0, float m22 = 1, float tx = 0, float ty = 0)
	{
        m_[0] = m11;
        m_[1] = m21;
        m_[4] = m12;
        m_[5] = m22;
		m_[12] = tx;
		m_[13] = ty;
		setType();
	}

	void transformPoint(float x, float y, float* newx, float* newy) const;
	void inverseTransformPoint(float x, float y, float* newx, float* newy) const;

	void setTx(float tx)
	{
		m_[12] = tx;
		setType();
	}

	void setTy(float ty)
	{
		m_[13] = ty;
		setType();
	}

	float tx() const
	{
		return m_[12];
	}

	float ty() const
	{
		return m_[13];
	}

    float m11() const
	{
		return m_[0];
	}

    float m21() const
	{
		return m_[1];
	}

    float m12() const
	{
		return m_[4];
	}

    float m22() const
	{
		return m_[5];
	}

    void setM11(float m11)
	{
        m_[0] = m11;
		setType();
	}

    void setM21(float m21)
	{
        m_[1] = m21;
		setType();
	}

    void setM12(float m12)
	{
        m_[4] = m12;
		setType();
	}

    void setM22(float m22)
	{
        m_[5] = m22;
		setType();
	}

	const float* data() const
	{
		return m_;
	}

	Type type() const
	{
		return type_;
	}

	friend Matrix2D operator*(const Matrix2D& m0, const Matrix2D& m1);
	friend bool operator==(const Matrix2D& m0, const Matrix2D& m1);
	friend bool operator!=(const Matrix2D& m0, const Matrix2D& m1);

	Matrix2D inverse() const;

private:
	float m_[16];
    // old
	//	a,  b,  0, 0,
	//	c,  d,  0, 0,
	//	0,  0,  1, 0,
	//	tx, ty, 0, 1

    // new
    //	m11, m21, 0, 0,
    //	m12, m22, 0, 0,
    //	0,   0,   1, 0,
    //	tx,  ty,  0, 1


	void setType();
	Type type_;
};

#endif
