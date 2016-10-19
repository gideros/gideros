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

void Transform::setMatrix(const float *m)
{
	matrix_.set(m);
	rotationZ_=0;
	rotationX_=0;
	rotationY_=0;
	scaleX_=1.0;
	scaleY_=1.0;
	scaleZ_=1.0;

	tx_=m[12];
	ty_=m[13];
	tz_=m[14];

    refX_=0;
	refY_=0;
	refZ_=0;
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
	scaleZ_=1.0;

	tx_=tx;
	ty_=ty;
	tz_=0;

        refX_=0;
	refY_=0;
	refZ_=0;

	float *mm=matrix_.raw();
    mm[15] = 1.0f;
    mm[2] = mm[3] = mm[6] = mm[7] = mm[8] = mm[9] = mm[11] = 0.0f;
    mm[0]=m11; mm[1]=m12; mm[4]=m21; mm[5]=m22; mm[10]=1.0f;
    mm[12]=tx; mm[13]=ty; mm[14]=0.0f; mm[15]=1.0f;
    matrix_.type=Matrix4::M2D;


    //compose();
}

void Transform::compose()
{
#if 1
	const float DEG2RAD = 3.141593f / 180;
	// Perform straight scale/rotate/translate
	/*matrix_.identity();
	matrix_.translate(-refX_,-refY_,-refZ_); //12 Mult
	matrix_.scale(scaleX_,scaleY_,scaleZ_); //12 Mult
	*/
	float *m=matrix_.raw();
    m[15] = 1.0f;
    m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = 0.0f;
    m[0]=scaleX_; m[5]=scaleY_; m[10]=scaleZ_;
    m[12]=-refX_*scaleX_; m[13]=-refY_*scaleY_; m[14]=-refZ_*scaleZ_;
    matrix_.type=Matrix4::TRANSLATE;
    if ((scaleX_!=1)||(scaleY_!=1))
        matrix_.type=Matrix4::M2D;
    if (scaleZ_!=1)
        matrix_.type=Matrix4::M3D;
	/* sx		0			0			0
	 * 0   		sy  		0  			0
	 * 0		0			sz			0
	 * -refx*sx	-refy*sy	-refz*sz	1
	 */
    if (rotationZ_||skewX_||skewY_)
	{
		//matrix_.rotateZ(rotationZ_);
	    float c = cosf(rotationZ_ * DEG2RAD);
	    float s = sinf(rotationZ_ * DEG2RAD);
        float kx = tanf(skewX_ * DEG2RAD);
        float ky = tanf(skewY_ * DEG2RAD);
	    float m0 = m[0],  m1 = m[1],
	          m4 = m[4],  m5 = m[5],
	          m8 = m[8],  m9 = m[9],
	          m12= m[12], m13= m[13];

	    // First rot: m1, m2, m4, m6, m8 and m9 are zero

        m[0] = c * m0 - ky * s * m5;
        m[1] = s * m0 + ky * c * m5;
        m[4] = kx * c * m0 - s * m5;
        m[5] = kx * s * m0 + c * m5;
        m[12]= c * m12 - s * m13;
        m[13]= s * m12 + c * m13;
	    if (matrix_.type==Matrix4::TRANSLATE)
	    	matrix_.type=Matrix4::M2D;
	}
	if (rotationY_)
	{
		//matrix_.rotateY(rotationY_);
	    float c = cosf(rotationY_ * DEG2RAD);
	    float s = sinf(rotationY_ * DEG2RAD);
	    float m0 = m[0],  m2 = m[2],
	          m4 = m[4],  m6 = m[6],
	          m8 = m[8],  m10= m[10],
	          m12= m[12], m14= m[14];

	    // Second rot: m2, m6, m8 and m9 are still zero
	    m[0] = m0 * c /*+ m2 * s*/;
	    m[2] = m0 *-s /*+ m2 * c*/;
	    m[4] = m4 * c /*+ m6 * s*/;
	    m[6] = m4 *-s /*+ m6 * c*/;
	    m[8] = /*m8 * c + */m10* s;
	    m[10]= /*m8 *-s + */m10* c;
	    m[12]= m12* c + m14* s;
	    m[14]= m12*-s + m14* c;
    	matrix_.type=Matrix4::M3D;
	}
	if (rotationX_)
	{
		//matrix_.rotateX(rotationX_);
	    float c = cosf(rotationX_ * DEG2RAD);
	    float s = sinf(rotationX_ * DEG2RAD);
	    float m1 = m[1],  m2 = m[2],
	          m5 = m[5],  m6 = m[6],
	          m9 = m[9],  m10= m[10],
	          m13= m[13], m14= m[14];

	    // Third rot: m9 is still zero
	    m[1] = m1 * c + m2 *-s;
	    m[2] = m1 * s + m2 * c;
	    m[5] = m5 * c + m6 *-s;
	    m[6] = m5 * s + m6 * c;
	    m[9] = /*m9 * c +*/ m10*-s;
	    m[10]= /*m9 * s +*/ m10* c;
	    m[13]= m13* c + m14*-s;
	    m[14]= m13* s + m14* c;
    	matrix_.type=Matrix4::M3D;
	}
	//matrix_.translate(tx_,ty_,tz_); //12 Mult
	m[12]+=tx_;	m[13]+=ty_;	m[14]+=tz_;
	isDirty_=false;

#else
	// optimized version of the code above
	// {{cos(a),-sin(a)},{sin(a),cos(a)}} * {{x, u * y},{0,v * y}}
	float c = cos(rotation_ * M_PI / 180);
	float s = sin(rotation_ * M_PI / 180);


#endif

}
