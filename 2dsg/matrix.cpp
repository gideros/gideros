#include "matrix.h"

void Matrix2D::transformPoint(float x, float y, float* newx, float* newy) const
{
    float nx = m11() * x + m12() * y + tx();
    float ny = m21() * x + m22() * y + ty();

	if (newx)
		*newx = nx;

	if (newy)
		*newy = ny;
}

void Matrix2D::inverseTransformPoint(float x, float y, float* newx, float* newy) const
{
    float invdet = 1 / (m11() * m22() - m21() * m12());

    float nx = invdet * m22() * (x - tx()) + invdet * -m12() * (y - ty());
    float ny = invdet * -m21() * (x - tx()) + invdet * m11() * (y - ty());

	if (newx)
		*newx = nx;

	if (newy)
		*newy = ny;
}

Matrix2D Matrix2D::inverse() const
{
	Matrix2D T(1, 0, 0, 1, -tx(), -ty());

    float invdet = 1 / (m11() * m22() - m21() * m12());
    Matrix2D L(m22() * invdet, -m12() * invdet, -m21() * invdet, m11() * invdet);

	return L * T;
}

Matrix2D operator*(const Matrix2D& m0, const Matrix2D& m1)
{
    float m11 = m0.m11() * m1.m11() + m0.m12() * m1.m21();
    float m12 = m0.m11() * m1.m12() + m0.m12() * m1.m22();
    float m21 = m0.m21() * m1.m11() + m0.m22() * m1.m21();
    float m22 = m0.m21() * m1.m12() + m0.m22() * m1.m22();
    float tx  = m0.m11() * m1.tx() + m0.m12() * m1.ty() + m0.tx();
    float ty  = m0.m21() * m1.tx() + m0.m22() * m1.ty() + m0.ty();

    return Matrix2D(m11, m12, m21, m22, tx, ty);
}

void Matrix2D::setType()
{
	bool I =	fabs(m_[0] - 1) < 1e-6f &&
				fabs(m_[1] - 0) < 1e-6f &&
				fabs(m_[4] - 0) < 1e-6f &&
				fabs(m_[5] - 1) < 1e-6f;

	bool tI =	fabs(m_[12] - 0) < 1e-6f &&
				fabs(m_[13] - 0) < 1e-6f;
				

	if (I == true && tI == true)
		type_ = eIdentity;
	else if (I == true)
		type_ = eTranslationOnly;
	else
		type_ = eArbitrary;
}

bool operator==(const Matrix2D& m0, const Matrix2D& m1)
{
	for (int i = 0; i < 16; ++i)
		if (!(m0.m_[i] == m1.m_[i]))
			return false;

	return true;
}

bool operator!=(const Matrix2D& m0, const Matrix2D& m1)
{
	for (int i = 0; i < 16; ++i)
		if (m0.m_[i] != m1.m_[i])
			return true;

	return false;
}
