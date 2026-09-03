///////////////////////////////////////////////////////////////////////////////
// Matrice.cpp
// ===========
// NxN Matrix Math classes
//
// The elements of the matrix are stored as column major order.
// | 0 2 |    | 0 3 6 |    |  0  4  8 12 |
// | 1 3 |    | 1 4 7 |    |  1  5  9 13 |
//            | 2 5 8 |    |  2  6 10 14 |
//                         |  3  7 11 15 |
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2005-06-24
// UPDATED: 2014-09-21
//
// Copyright (C) 2005 Song Ho Ahn
///////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <algorithm>
#include "Matrices.h"
#include "glog.h"

const float DEG2RAD = 3.141593f / 180;
const float EPSILON = 0.00001f;



///////////////////////////////////////////////////////////////////////////////
// transpose 2x2 matrix
///////////////////////////////////////////////////////////////////////////////
Matrix2& Matrix2::transpose()
{
    std::swap(m[1],  m[2]);
    return *this;
}



///////////////////////////////////////////////////////////////////////////////
// return the determinant of 2x2 matrix
///////////////////////////////////////////////////////////////////////////////
float Matrix2::getDeterminant()
{
    return m[0] * m[3] - m[1] * m[2];
}



///////////////////////////////////////////////////////////////////////////////
// inverse of 2x2 matrix
// If cannot find inverse, set identity matrix
///////////////////////////////////////////////////////////////////////////////
Matrix2& Matrix2::invert()
{
    float determinant = getDeterminant();
/*    if(fabs(determinant) ==0)
    	determinant=EPSILON;*/

    float tmp = m[0];   // copy the first element
    float invDeterminant = 1.0f / determinant;
    m[0] =  invDeterminant * m[3];
    m[1] = -invDeterminant * m[1];
    m[2] = -invDeterminant * m[2];
    m[3] =  invDeterminant * tmp;

    return *this;
}



///////////////////////////////////////////////////////////////////////////////
// transpose 3x3 matrix
///////////////////////////////////////////////////////////////////////////////
Matrix3& Matrix3::transpose()
{
    std::swap(m[1],  m[3]);
    std::swap(m[2],  m[6]);
    std::swap(m[5],  m[7]);

    return *this;
}



///////////////////////////////////////////////////////////////////////////////
// return determinant of 3x3 matrix
///////////////////////////////////////////////////////////////////////////////
float Matrix3::getDeterminant()
{
    return m[0] * (m[4] * m[8] - m[5] * m[7]) -
           m[1] * (m[3] * m[8] - m[5] * m[6]) +
           m[2] * (m[3] * m[7] - m[4] * m[6]);
}



///////////////////////////////////////////////////////////////////////////////
// inverse 3x3 matrix
// If cannot find inverse, set identity matrix
///////////////////////////////////////////////////////////////////////////////
Matrix3& Matrix3::invert()
{
    float determinant, invDeterminant;
    float tmp[9];

    tmp[0] = m[4] * m[8] - m[5] * m[7];
    tmp[1] = m[2] * m[7] - m[1] * m[8];
    tmp[2] = m[1] * m[5] - m[2] * m[4];
    tmp[3] = m[5] * m[6] - m[3] * m[8];
    tmp[4] = m[0] * m[8] - m[2] * m[6];
    tmp[5] = m[2] * m[3] - m[0] * m[5];
    tmp[6] = m[3] * m[7] - m[4] * m[6];
    tmp[7] = m[1] * m[6] - m[0] * m[7];
    tmp[8] = m[0] * m[4] - m[1] * m[3];

    // check determinant if it is 0
    determinant = m[0] * tmp[0] + m[1] * tmp[3] + m[2] * tmp[6];
/*    if(fabs(determinant) ==0)
    	determinant=EPSILON;*/

    // divide by the determinant
    invDeterminant = 1.0f / determinant;
    m[0] = invDeterminant * tmp[0];
    m[1] = invDeterminant * tmp[1];
    m[2] = invDeterminant * tmp[2];
    m[3] = invDeterminant * tmp[3];
    m[4] = invDeterminant * tmp[4];
    m[5] = invDeterminant * tmp[5];
    m[6] = invDeterminant * tmp[6];
    m[7] = invDeterminant * tmp[7];
    m[8] = invDeterminant * tmp[8];

    return *this;
}



///////////////////////////////////////////////////////////////////////////////
// transpose 4x4 matrix
///////////////////////////////////////////////////////////////////////////////
Matrix4& Matrix4::transpose()
{
    std::swap(m[1],  m[4]);
    std::swap(m[2],  m[8]);
    std::swap(m[3],  m[12]);
    std::swap(m[6],  m[9]);
    std::swap(m[7],  m[13]);
    std::swap(m[11], m[14]);

    return *this;
}


void Matrix4::transformPoint(float x, float y, float* newx, float* newy) const
{
    switch(type) {
    case TRANSLATE:
        *newx = x + m[12];
        *newy = y + m[13];
        break;
    case M2D:
    case M3D:
    case FULL:
        *newx = m[0]*x + m[4]*y + m[12];
        *newy = m[1]*x + m[5]*y + m[13];
        break;
    }
}

void Matrix4::inverseTransformPoint(float x, float y, float* newx, float* newy) const
{
    switch(type) {
    case TRANSLATE:
        *newx = x - m[12];
        *newy = y - m[13];
        break;
    case M2D: {
        // Fast inverse for 2D affine: | a c tx |^-1
        //                             | b d ty |
        //                             | 0 0 1  |
        float det = m[0]*m[5] - m[1]*m[4];
        if (fabs(det) > 1e-6f) {
            float invDet = 1.0f / det;
            float dx = x - m[12];
            float dy = y - m[13];
            *newx = invDet * (m[5]*dx - m[4]*dy);
            *newy = invDet * (m[0]*dy - m[1]*dx);
        } else {
            *newx = x;
            *newy = y;
        }
        break;
    }
    case M3D:
    case FULL:
    {
        Matrix4 inv=inverse();
        const float *m=inv.data();
        *newx = m[0]*x + m[4]*y + m[12];
        *newy = m[1]*x + m[5]*y + m[13];
        break;
    }
    }
}

void Matrix4::transformPoint(float x, float y, float z, float* newx, float* newy, float* newz) const
{
    switch(type) {
    case TRANSLATE:
        *newx = x + m[12];
        *newy = y + m[13];
        *newz = z + m[14];
        break;
    case M2D:
        *newx = m[0]*x + m[4]*y + m[12];
        *newy = m[1]*x + m[5]*y + m[13];
        *newz = z + m[14];
        break;
    case M3D:
    case FULL:
        *newx = m[0]*x + m[4]*y + m[8]*z + m[12];
        *newy = m[1]*x + m[5]*y + m[9]*z + m[13];
        *newz = m[2]*x + m[6]*y + m[10]*z + m[14];
        break;
    }
}

void Matrix4::inverseTransformPoint(float x, float y, float z,float* newx, float* newy, float* newz) const
{
	Matrix4 inv=inverse();
    const float *m=inv.data();
    switch(type) {
    case TRANSLATE:
        *newx = x + m[12];
        *newy = y + m[13];
        *newz = z + m[14];
        break;
    case M2D:
        *newx = m[0]*x + m[4]*y + m[12];
        *newy = m[1]*x + m[5]*y + m[13];
        *newz = z + m[14];
        break;
    case M3D:
    case FULL:
        *newx = m[0]*x + m[4]*y + m[8]*z + m[12];
        *newy = m[1]*x + m[5]*y + m[9]*z + m[13];
        *newz = m[2]*x + m[6]*y + m[10]*z + m[14];
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// inverse 4x4 matrix
///////////////////////////////////////////////////////////////////////////////
Matrix4& Matrix4::invert()
{
    switch (type) {
    case TRANSLATE:
        m[12] = -m[12];
        m[13] = -m[13];
        m[14] = -m[14];
        break;
    case M2D:
    {
        float a=m[0], b=m[1];
        float c=m[4], d=m[5];
        float tx=m[12], ty=m[13], tz=m[14];

        float det = a*d - b*c;
        float invDet = 1.0f / det;

        // R^-1
        m[0] =  d * invDet;
        m[1] = -b * invDet;
        m[4] = -c * invDet;
        m[5] =  a * invDet;

        // -R^-1 * T
        m[12] = -(m[0]*tx + m[4]*ty);
        m[13] = -(m[1]*tx + m[5]*ty);
        m[14] = -tz;

        m[3]=0; m[7]=0; m[11]=0; m[15]=1;
        break;
    }
    case M3D:
    {
        float r0=m[0], r1=m[1], r2=m[2];
        float r4=m[4], r5=m[5], r6=m[6];
        float r8=m[8], r9=m[9], r10=m[10];

        float tx=m[12], ty=m[13], tz=m[14];

        float det =
            r0*(r5*r10 - r6*r9) -
            r1*(r4*r10 - r6*r8) +
            r2*(r4*r9  - r5*r8);

        float invDet = 1.0f / det;

        float i0 =  (r5*r10 - r6*r9) * invDet;
        float i1 = -(r1*r10 - r2*r9) * invDet;
        float i2 =  (r1*r6  - r2*r5) * invDet;

        float i4 = -(r4*r10 - r6*r8) * invDet;
        float i5 =  (r0*r10 - r2*r8) * invDet;
        float i6 = -(r0*r6  - r2*r4) * invDet;

        float i8 =  (r4*r9  - r5*r8) * invDet;
        float i9 = -(r0*r9  - r1*r8) * invDet;
        float i10=  (r0*r5  - r1*r4) * invDet;

        // R^-1
        m[0]=i0;  m[1]=i1;  m[2]=i2;
        m[4]=i4;  m[5]=i5;  m[6]=i6;
        m[8]=i8;  m[9]=i9;  m[10]=i10;

        // -R^-1 * T
        m[12] = -(i0*tx + i4*ty + i8*tz);
        m[13] = -(i1*tx + i5*ty + i9*tz);
        m[14] = -(i2*tx + i6*ty + i10*tz);

        m[3]=0; m[7]=0; m[11]=0; m[15]=1;

        type = M3D;
        break;
    }
    case FULL:
        this->invertGeneral();
    }

    return *this;
}

Matrix4 Matrix4::inverse() const
{
	Matrix4 inv=*this;
	inv.invert();
	return inv;
}




///////////////////////////////////////////////////////////////////////////////
// compute the inverse of 4x4 Euclidean transformation matrix
//
// Euclidean transformation is translation, rotation, and reflection.
// With Euclidean transform, only the position and orientation of the object
// will be changed. Euclidean transform does not change the shape of an object
// (no scaling). Length and angle are reserved.
//
// Use inverseAffine() if the matrix has scale and shear transformation.
//
// M = [ R | T ]
//     [ --+-- ]    (R denotes 3x3 rotation/reflection matrix)
//     [ 0 | 1 ]    (T denotes 1x3 translation matrix)
//
// y = M*x  ->  y = R*x + T  ->  x = R^-1*(y - T)  ->  x = R^T*y - R^T*T
// (R is orthogonal,  R^-1 = R^T)
//
//  [ R | T ]-1    [ R^T | -R^T * T ]    (R denotes 3x3 rotation matrix)
//  [ --+-- ]   =  [ ----+--------- ]    (T denotes 1x3 translation)
//  [ 0 | 1 ]      [  0  |     1    ]    (R^T denotes R-transpose)
///////////////////////////////////////////////////////////////////////////////
Matrix4& Matrix4::invertEuclidean()
{
    // transpose 3x3 rotation matrix part
    // | R^T | 0 |
    // | ----+-- |
    // |  0  | 1 |
    float tmp;
    tmp = m[1];  m[1] = m[4];  m[4] = tmp;
    tmp = m[2];  m[2] = m[8];  m[8] = tmp;
    tmp = m[6];  m[6] = m[9];  m[9] = tmp;

    // compute translation part -R^T * T
    // | 0 | -R^T x |
    // | --+------- |
    // | 0 |   0    |
    float x = m[12];
    float y = m[13];
    float z = m[14];
    m[12] = -(m[0] * x + m[4] * y + m[8] * z);
    m[13] = -(m[1] * x + m[5] * y + m[9] * z);
    m[14] = -(m[2] * x + m[6] * y + m[10]* z);

    // last row should be unchanged (0,0,0,1)

    return *this;
}



///////////////////////////////////////////////////////////////////////////////
// compute the inverse of a 4x4 affine transformation matrix
//
// Affine transformations are generalizations of Euclidean transformations.
// Affine transformation includes translation, rotation, reflection, scaling,
// and shearing. Length and angle are NOT preserved.
// M = [ R | T ]
//     [ --+-- ]    (R denotes 3x3 rotation/scale/shear matrix)
//     [ 0 | 1 ]    (T denotes 1x3 translation matrix)
//
// y = M*x  ->  y = R*x + T  ->  x = R^-1*(y - T)  ->  x = R^-1*y - R^-1*T
//
//  [ R | T ]-1   [ R^-1 | -R^-1 * T ]
//  [ --+-- ]   = [ -----+---------- ]
//  [ 0 | 1 ]     [  0   +     1     ]
///////////////////////////////////////////////////////////////////////////////
Matrix4& Matrix4::invertAffine()
{
    // R^-1
    Matrix3 r(m[0],m[1],m[2], m[4],m[5],m[6], m[8],m[9],m[10]);
    r.invert();
    m[0] = r[0];  m[1] = r[1];  m[2] = r[2];
    m[4] = r[3];  m[5] = r[4];  m[6] = r[5];
    m[8] = r[6];  m[9] = r[7];  m[10]= r[8];

    // -R^-1 * T
    float x = m[12];
    float y = m[13];
    float z = m[14];
    m[12] = -(r[0] * x + r[3] * y + r[6] * z);
    m[13] = -(r[1] * x + r[4] * y + r[7] * z);
    m[14] = -(r[2] * x + r[5] * y + r[8] * z);

    // last row should be unchanged (0,0,0,1)
    //m[3] = m[7] = m[11] = 0.0f;
    //m[15] = 1.0f;

    const float EPS = 1e-6f;
    type=M3D;
    if ((fabs(m[10]-1)<EPS)&&(fabs(m[2])<EPS)&&
        (fabs(m[6])<EPS)&&(fabs(m[8])<EPS)&&(fabs(m[9])<EPS))
    {
        type=M2D;
        if ((fabs(m[0]-1)<EPS)&&(fabs(m[5]-1)<EPS)&&
            (fabs(m[1])<EPS)&&(fabs(m[4])<EPS))
            type=TRANSLATE;
    }

    return * this;
}



///////////////////////////////////////////////////////////////////////////////
// inverse matrix using matrix partitioning (blockwise inverse)
// It devides a 4x4 matrix into 4 of 2x2 matrices. It works in case of where
// det(A) != 0. If not, use the generic inverse method
// inverse formula.
// M = [ A | B ]    A, B, C, D are 2x2 matrix blocks
//     [ --+-- ]    det(M) = |A| * |D - ((C * A^-1) * B)|
//     [ C | D ]
//
// M^-1 = [ A' | B' ]   A' = A^-1 - (A^-1 * B) * C'
//        [ ---+--- ]   B' = (A^-1 * B) * -D'
//        [ C' | D' ]   C' = -D' * (C * A^-1)
//                      D' = (D - ((C * A^-1) * B))^-1
//
// NOTE: I wrap with () if it it used more than once.
//       The matrix is invertable even if det(A)=0, so must check det(A) before
//       calling this function, and use invertGeneric() instead.
///////////////////////////////////////////////////////////////////////////////
Matrix4& Matrix4::invertProjective()
{
    // partition
    Matrix2 a(m[0], m[1], m[4], m[5]);
    Matrix2 b(m[8], m[9], m[12], m[13]);
    Matrix2 c(m[2], m[3], m[6], m[7]);
    Matrix2 d(m[10], m[11], m[14], m[15]);

    // pre-compute repeated parts
    a.invert();             // A^-1
    Matrix2 ab = a * b;     // A^-1 * B
    Matrix2 ca = c * a;     // C * A^-1
    Matrix2 cab = ca * b;   // C * A^-1 * B
    Matrix2 dcab = d - cab; // D - C * A^-1 * B

    // check determinant if |D - C * A^-1 * B| = 0
    //NOTE: this function assumes det(A) is already checked. if |A|=0 then,
    //      cannot use this function.
    float determinant = dcab[0] * dcab[3] - dcab[1] * dcab[2];
    if(fabs(determinant) <= EPSILON)
    {
        return identity();
    }

    // compute D' and -D'
    Matrix2 d1 = dcab;      //  (D - C * A^-1 * B)
    d1.invert();            //  (D - C * A^-1 * B)^-1
    Matrix2 d2 = -d1;       // -(D - C * A^-1 * B)^-1

    // compute C'
    Matrix2 c1 = d2 * ca;   // -D' * (C * A^-1)

    // compute B'
    Matrix2 b1 = ab * d2;   // (A^-1 * B) * -D'

    // compute A'
    Matrix2 a1 = a - (ab * c1); // A^-1 - (A^-1 * B) * C'

    // assemble inverse matrix
    m[0] = a1[0];  m[4] = a1[2]; /*|*/ m[8] = b1[0];  m[12]= b1[2];
    m[1] = a1[1];  m[5] = a1[3]; /*|*/ m[9] = b1[1];  m[13]= b1[3];
    /*-----------------------------+-----------------------------*/
    m[2] = c1[0];  m[6] = c1[2]; /*|*/ m[10]= d1[0];  m[14]= d1[2];
    m[3] = c1[1];  m[7] = c1[3]; /*|*/ m[11]= d1[1];  m[15]= d1[3];
    computeType();

    return *this;
}



///////////////////////////////////////////////////////////////////////////////
// compute the inverse of a general 4x4 matrix using Cramer's Rule
// If cannot find inverse, return indentity matrix
// M^-1 = adj(M) / det(M)
///////////////////////////////////////////////////////////////////////////////
Matrix4& Matrix4::invertGeneral()
{
    // get cofactors of minor matrices
    float cofactor0 = getCofactor(m[5],m[6],m[7], m[9],m[10],m[11], m[13],m[14],m[15]);
    float cofactor1 = getCofactor(m[4],m[6],m[7], m[8],m[10],m[11], m[12],m[14],m[15]);
    float cofactor2 = getCofactor(m[4],m[5],m[7], m[8],m[9], m[11], m[12],m[13],m[15]);
    float cofactor3 = getCofactor(m[4],m[5],m[6], m[8],m[9], m[10], m[12],m[13],m[14]);

    // get determinant
    float determinant = m[0] * cofactor0 - m[1] * cofactor1 + m[2] * cofactor2 - m[3] * cofactor3;
    if(fabs(determinant) <= EPSILON)
    {
        return identity();
    }

    // get rest of cofactors for adj(M)
    float cofactor4 = getCofactor(m[1],m[2],m[3], m[9],m[10],m[11], m[13],m[14],m[15]);
    float cofactor5 = getCofactor(m[0],m[2],m[3], m[8],m[10],m[11], m[12],m[14],m[15]);
    float cofactor6 = getCofactor(m[0],m[1],m[3], m[8],m[9], m[11], m[12],m[13],m[15]);
    float cofactor7 = getCofactor(m[0],m[1],m[2], m[8],m[9], m[10], m[12],m[13],m[14]);

    float cofactor8 = getCofactor(m[1],m[2],m[3], m[5],m[6], m[7],  m[13],m[14],m[15]);
    float cofactor9 = getCofactor(m[0],m[2],m[3], m[4],m[6], m[7],  m[12],m[14],m[15]);
    float cofactor10= getCofactor(m[0],m[1],m[3], m[4],m[5], m[7],  m[12],m[13],m[15]);
    float cofactor11= getCofactor(m[0],m[1],m[2], m[4],m[5], m[6],  m[12],m[13],m[14]);

    float cofactor12= getCofactor(m[1],m[2],m[3], m[5],m[6], m[7],  m[9], m[10],m[11]);
    float cofactor13= getCofactor(m[0],m[2],m[3], m[4],m[6], m[7],  m[8], m[10],m[11]);
    float cofactor14= getCofactor(m[0],m[1],m[3], m[4],m[5], m[7],  m[8], m[9], m[11]);
    float cofactor15= getCofactor(m[0],m[1],m[2], m[4],m[5], m[6],  m[8], m[9], m[10]);

    // build inverse matrix = adj(M) / det(M)
    // adjugate of M is the transpose of the cofactor matrix of M
    float invDeterminant = 1.0f / determinant;
    m[0] =  invDeterminant * cofactor0;
    m[1] = -invDeterminant * cofactor4;
    m[2] =  invDeterminant * cofactor8;
    m[3] = -invDeterminant * cofactor12;

    m[4] = -invDeterminant * cofactor1;
    m[5] =  invDeterminant * cofactor5;
    m[6] = -invDeterminant * cofactor9;
    m[7] =  invDeterminant * cofactor13;

    m[8] =  invDeterminant * cofactor2;
    m[9] = -invDeterminant * cofactor6;
    m[10]=  invDeterminant * cofactor10;
    m[11]= -invDeterminant * cofactor14;

    m[12]= -invDeterminant * cofactor3;
    m[13]=  invDeterminant * cofactor7;
    m[14]= -invDeterminant * cofactor11;
    m[15]=  invDeterminant * cofactor15;
    computeType();

    return *this;
}



///////////////////////////////////////////////////////////////////////////////
// return determinant of 4x4 matrix
///////////////////////////////////////////////////////////////////////////////
float Matrix4::getDeterminant()
{
    return m[0] * getCofactor(m[5],m[6],m[7], m[9],m[10],m[11], m[13],m[14],m[15]) -
           m[1] * getCofactor(m[4],m[6],m[7], m[8],m[10],m[11], m[12],m[14],m[15]) +
           m[2] * getCofactor(m[4],m[5],m[7], m[8],m[9], m[11], m[12],m[13],m[15]) -
           m[3] * getCofactor(m[4],m[5],m[6], m[8],m[9], m[10], m[12],m[13],m[14]);
}



///////////////////////////////////////////////////////////////////////////////
// compute cofactor of 3x3 minor matrix without sign
// input params are 9 elements of the minor matrix
// NOTE: The caller must know its sign.
///////////////////////////////////////////////////////////////////////////////
float Matrix4::getCofactor(float m0, float m1, float m2,
                           float m3, float m4, float m5,
                           float m6, float m7, float m8)
{
    return m0 * (m4 * m8 - m5 * m7) -
           m1 * (m3 * m8 - m5 * m6) +
           m2 * (m3 * m7 - m4 * m6);
}



///////////////////////////////////////////////////////////////////////////////
// translate this matrix by (x, y, z)
///////////////////////////////////////////////////////////////////////////////
Matrix4& Matrix4::translate(const Vector3& v)
{
    return translate(v.x, v.y, v.z);
}

Matrix4& Matrix4::translate(float x, float y, float z)
{
    switch (type)
    {
    case TRANSLATE:
    case M2D:
    case M3D:
        m[12]+= x; m[13]+=y; m[14]+=z;
        break;
    case FULL:
        m[0] += m[3] * x;   m[4] += m[7] * x;   m[8] += m[11]* x;   m[12]+= m[15]* x;
        m[1] += m[3] * y;   m[5] += m[7] * y;   m[9] += m[11]* y;   m[13]+= m[15]* y;
        m[2] += m[3] * z;   m[6] += m[7] * z;   m[10]+= m[11]* z;   m[14]+= m[15]* z;
        break;
    }

    return *this;
}



///////////////////////////////////////////////////////////////////////////////
// uniform scale
///////////////////////////////////////////////////////////////////////////////
Matrix4& Matrix4::scale(float s)
{
    return scale(s, s, s);
}

Matrix4& Matrix4::scale(float x, float y, float z)
{
    switch (type)
    {
    case TRANSLATE:
        m[0] = x;   m[12] *= x;
        m[5] = y;   m[13] *= y;
        m[10]= z;   m[14] *= z;
        type=(z!=1)?M3D:M2D;
        break;
    case M2D:
        m[0] *= x;   m[4] *= x;   m[12] *= x;
        m[1] *= y;   m[5] *= y;   m[13] *= y;
        m[10]= z;                 m[14] *= z;
        type=(z!=1)?M3D:M2D;
        break;
    case M3D:
    default:
        m[0] *= x;   m[4] *= x;   m[8] *= x;   m[12] *= x;
        m[1] *= y;   m[5] *= y;   m[9] *= y;   m[13] *= y;
        m[2] *= z;   m[6] *= z;   m[10]*= z;   m[14] *= z;
        break;
    }
    return *this;
}



///////////////////////////////////////////////////////////////////////////////
// build a rotation matrix with given angle(degree) and rotation axis, then
// multiply it with this object
///////////////////////////////////////////////////////////////////////////////
Matrix4& Matrix4::rotate(float angle, const Vector3& axis)
{
    return rotate(angle, axis.x, axis.y, axis.z);
}


Matrix4& Matrix4::rotate(float angle, float x, float y, float z)
{
    float c = cosf(angle * DEG2RAD);
    float s = sinf(angle * DEG2RAD);
    float c1 = 1.0f - c;

    bool Zaxis=x == 0 && y == 0 && z == 1;
    switch (type)
    {
    // ---------------------------------------------------------
    // CASE 1 : TRANSLATE → devient M3D
    // ---------------------------------------------------------
    case TRANSLATE:
    {
        type = M2D; //Will become M2D on all cases
        // Rotation autour de Z ?
        if (Zaxis)
        {
            // Rotation 2D
            float tx = m[12], ty = m[13];

            // Colonnes X/Y deviennent la rotation 2D
            m[0] = c;   m[1] = s;
            m[4] = -s;  m[5] = c;

            // Translation tournée en 2D
            m[12] = c*tx - s*ty;
            m[13] = s*tx + c*ty;

            // Z reste inchangé
            // W reste inchangé

            break;
        }

        // Sinon → rotation 3D → devient M3D
        // (on retombe sur le cas M3D)
    }
    case M2D:
    {
        // Rotation 2D uniquement si axe = Z
        if (Zaxis)
        {
            float m0 = m[0], m4 = m[4], m12 = m[12];
            float m1 = m[1], m5 = m[5], m13 = m[13];

            // Rotation dans le plan XY
            m[0] = c*m0 - s*m1;
            m[1] = s*m0 + c*m1;

            m[4] = c*m4 - s*m5;
            m[5] = s*m4 + c*m5;

            // Translation 2D
            m[12] = c*m12 - s*m13;
            m[13] = s*m12 + c*m13;

            // Z reste inchangé
            break;
        }

        type = M3D; //Will become M3D
        // Sinon → rotation 3D → devient M3D
        // On retombe sur le cas M3D ci-dessous
    }
    case M3D:
    case FULL:
    {
        float m0 = m[0],  m4 = m[4],  m8 = m[8],  m12= m[12];
        float m1 = m[1],  m5 = m[5],  m9 = m[9],  m13= m[13];
        float m2 = m[2],  m6 = m[6],  m10= m[10], m14= m[14];

        float r0 = x*x*c1 + c;
        float r1 = x*y*c1 + z*s;
        float r2 = x*z*c1 - y*s;

        float r4 = x*y*c1 - z*s;
        float r5 = y*y*c1 + c;
        float r6 = y*z*c1 + x*s;

        float r8 = x*z*c1 + y*s;
        float r9 = y*z*c1 - x*s;
        float r10= z*z*c1 + c;

        m[0] = r0*m0 + r4*m1 + r8*m2;
        m[1] = r1*m0 + r5*m1 + r9*m2;
        m[2] = r2*m0 + r6*m1 + r10*m2;

        m[4] = r0*m4 + r4*m5 + r8*m6;
        m[5] = r1*m4 + r5*m5 + r9*m6;
        m[6] = r2*m4 + r6*m5 + r10*m6;

        m[8] = r0*m8 + r4*m9 + r8*m10;
        m[9] = r1*m8 + r5*m9 + r9*m10;
        m[10]= r2*m8 + r6*m9 + r10*m10;

        m[12]= r0*m12+ r4*m13+ r8*m14;
        m[13]= r1*m12+ r5*m13+ r9*m14;
        m[14]= r2*m12+ r6*m13+ r10*m14;

        break;
    }
    }

    return *this;
}

Matrix4& Matrix4::rotateX(float angle)
{
    float c = cosf(angle * DEG2RAD);
    float s = sinf(angle * DEG2RAD);
    float m1 = m[1],  m2 = m[2],
          m5 = m[5],  m6 = m[6],
          m9 = m[9],  m10= m[10],
          m13= m[13], m14= m[14];

    m[1] = m1 * c + m2 *-s;
    m[2] = m1 * s + m2 * c;
    m[5] = m5 * c + m6 *-s;
    m[6] = m5 * s + m6 * c;
    m[9] = m9 * c + m10*-s;
    m[10]= m9 * s + m10* c;
    m[13]= m13* c + m14*-s;
    m[14]= m13* s + m14* c;
    computeType();

    return *this;
}

Matrix4& Matrix4::rotateY(float angle)
{
    float c = cosf(angle * DEG2RAD);
    float s = sinf(angle * DEG2RAD);
    float m0 = m[0],  m2 = m[2],
          m4 = m[4],  m6 = m[6],
          m8 = m[8],  m10= m[10],
          m12= m[12], m14= m[14];

    m[0] = m0 * c + m2 * s;
    m[2] = m0 *-s + m2 * c;
    m[4] = m4 * c + m6 * s;
    m[6] = m4 *-s + m6 * c;
    m[8] = m8 * c + m10* s;
    m[10]= m8 *-s + m10* c;
    m[12]= m12* c + m14* s;
    m[14]= m12*-s + m14* c;
    computeType();

    return *this;
}

Matrix4& Matrix4::rotateZ(float angle)
{
    float c = cosf(angle * DEG2RAD);
    float s = sinf(angle * DEG2RAD);
    float m0 = m[0],  m1 = m[1],
          m4 = m[4],  m5 = m[5],
          m8 = m[8],  m9 = m[9],
          m12= m[12], m13= m[13];

    m[0] = m0 * c + m1 *-s;
    m[1] = m0 * s + m1 * c;
    m[4] = m4 * c + m5 *-s;
    m[5] = m4 * s + m5 * c;
    m[8] = m8 * c + m9 *-s;
    m[9] = m8 * s + m9 * c;
    m[12]= m12* c + m13*-s;
    m[13]= m12* s + m13* c;
    computeType();
    return *this;
}
