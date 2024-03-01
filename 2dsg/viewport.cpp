#include "viewport.h"
#include "ogl.h"
#include "application.h"

Viewport::Viewport(Application* application) : Sprite(application)
{
	content_ = NULL;
    target_ = NULL;
	hasProjection_=false;
	captureMode_=false;
}

void Viewport::cloneFrom(Viewport *s) {
    Sprite::cloneFrom(s);

    matrix_=s->matrix_;
    projection_=s->projection_;
    hasProjection_=s->hasProjection_;
    setContent(s->content_);
    setTarget(s->target_,s->captureMode_);
}

Viewport::~Viewport()
{
    setContent(NULL);
    setTarget(NULL,false);
}

void Viewport::setContent(Sprite *s)
{
    if (s) {
        s->ref();
        if (!s->viewports)
            s->viewports=new std::vector<Sprite *>();
        s->viewports->push_back(this);
    }
    if (content_&&content_->viewports) {
        content_->viewports->erase(std::find(content_->viewports->begin(),content_->viewports->end(),this));
        content_->unref();
    }
    content_ = s;
}

void Viewport::setTarget(GRenderTarget *s, bool capture)
{
    if (s)
        s->ref();
    if (target_)
        target_->unref();
    target_ = s;
    captureMode_=capture;
}

void Viewport::setTransform(const Matrix4* matrix)
{
	if (matrix)
		matrix_=*matrix;
	else
		matrix_.identity();
}

void Viewport::setProjection(const Matrix4* matrix)
{
	if (matrix)
	{
		projection_=*matrix;
		hasProjection_=true;
	}
	else
	{
		projection_.identity();
		hasProjection_=false;
	}
}

void Viewport::doDraw(const CurrentTransform&t, float sx, float sy, float ex, float ey)
{
	if (content_)
	{
        if (target_) {
            GRenderTarget *rt=target_;
            rt->clear(0,0,0,0,-1,-1,true,captureMode_);
            if (captureMode_)
				rt->draw(content_,t,true,true);
            else {
				target_=NULL; //Avoid reentrancy
				rt->draw(this,Matrix4(),true);
				target_=rt;
            }
        }
        else if (hasProjection_)
		{
			ShaderEngine::DepthStencil dp=ShaderEngine::Engine->pushDepthStencil();
			dp.dClear=true;
			ShaderEngine::Engine->setDepthStencil(dp);
			Matrix4 oldProj=ShaderEngine::Engine->getProjection();
			Matrix4 oldView=ShaderEngine::Engine->getView();
            Matrix4 np=oldProj;
            np.scale(1,1,-1); //Defeat ortho projection
            np=(np*t)*projection_;
			ShaderEngine::Engine->setProjection(np);
			ShaderEngine::Engine->setView(matrix_);
			Matrix4 id;
			((Sprite*)content_)->draw(id, sx,sy,ex,ey);
			ShaderEngine::Engine->setProjection(oldProj);
			ShaderEngine::Engine->setView(oldView);
			ShaderEngine::Engine->popDepthStencil();
		}
		else
			((Sprite*)content_)->draw(t*matrix_, sx,sy,ex,ey);
	}
}

void Viewport::lookAt(float eyex, float eyey, float eyez, float centerx,
      float centery, float centerz, float upx, float upy,
      float upz)
{
    Vector3 pos(eyex,eyey,eyez);
    Vector3 target(centerx,centery,centerz);
    Vector3 upDir(upx,upy,upz);

    // compute left/up/forward axis vectors
    Vector3 forward = (pos - target).normalize();
    Vector3 left = upDir.cross(forward).normalize();
    Vector3 up = forward.cross(left);

    float *m=matrix_.raw();
    // compute M = Mr * Mt
    m[0] = left.x;    m[4] = left.y;    m[8] = left.z;    m[12]= left.dot(-pos);
    m[1] = up.x;      m[5] = up.y;      m[9] = up.z;      m[13]= up.dot(-pos);
    m[2] = forward.x; m[6] = forward.y; m[10]= forward.z; m[14]= forward.dot(-pos);
    m[3] = 0.0f;      m[7] = 0.0f;      m[11]= 0.0f;      m[15] = 1.0f;

    matrix_.type=Matrix4::M3D;
}

void Viewport::lookAngles(float eyex, float eyey, float eyez,float ax, float ay, float az)
{
    Vector3 pos(eyex,eyey,eyez);
	Vector3 angles(ax,ay,az);
	Vector3 left,up,forward;

    const float DEG2RAD = 3.141593f / 180;
    float sx, sy, sz, cx, cy, cz, theta;

    // rotation angle about X-axis (pitch)
    theta = angles.x * DEG2RAD;
    sx = sinf(theta);
    cx = cosf(theta);

    // rotation angle about Y-axis (yaw)
    theta = angles.y * DEG2RAD;
    sy = sinf(theta);
    cy = cosf(theta);

    // rotation angle about Z-axis (roll)
    theta = angles.z * DEG2RAD;
    sz = sinf(theta);
    cz = cosf(theta);

    // determine left axis
    left.x = cy*cz;
    left.y = sx*sy*cz + cx*sz;
    left.z = -cx*sy*cz + sx*sz;

    // determine up axis
    up.x = -cy*sz;
    up.y = -sx*sy*sz + cx*cz;
    up.z = cx*sy*sz + sx*cz;

    // determine forward axis
    forward.x = sy;
    forward.y = -sx*cy;
    forward.z = cx*cy;

    float *m=matrix_.raw();
    // compute M = Mr * Mt
    m[0] = left.x;    m[4] = left.y;    m[8] = left.z;    m[12]= left.dot(-pos);
    m[1] = up.x;      m[5] = up.y;      m[9] = up.z;      m[13]= up.dot(-pos);
    m[2] = forward.x; m[6] = forward.y; m[10]= forward.z; m[14]= forward.dot(-pos);
    m[3] = 0.0f;      m[7] = 0.0f;      m[11]= 0.0f;      m[15] = 1.0f;

    matrix_.type=Matrix4::M3D;
}
