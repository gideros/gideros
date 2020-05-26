/*
 * lqSprites.cpp
 *
 *  Created on: 22 oct. 2017
 *      Author: Nicolas
 */

#include "lqSprites.h"

static void b2DG_Draw(void *c,const CurrentTransform&t, float sx, float sy, float ex, float ey)
{
	((b2DebugDraw *)c)->doDraw(t,sx, sy, ex, ey);
}

static void b2DG_Destroy(void *c)
{
	delete ((b2DebugDraw *)c);
}


b2DebugDraw::b2DebugDraw(LuaApplication* application) :
    world_(NULL),
    application_(application)
{
	proxy_=gtexture_get_spritefactory()->createProxy(application->getApplication(), this, b2DG_Draw, b2DG_Destroy);
	SetFlags(e_shapeBit);
}

b2DebugDraw::~b2DebugDraw()
{
}

void b2DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	glPushColor();
	glMultColor(color.r, color.g, color.b,1);
	gtexture_get_engine()->getDefault(ShaderEngine::STDP_BASIC)->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT, 2,vertices, vertexCount, true, NULL);
	gtexture_get_engine()->getDefault(ShaderEngine::STDP_BASIC)->drawArrays(ShaderProgram::LineLoop, 0, vertexCount);
	glPopColor();
}

void b2DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	gtexture_get_engine()->getDefault(ShaderEngine::STDP_BASIC)->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT,2, vertices, vertexCount, true, NULL);

	uint16_t *indices=new uint16_t[vertexCount];
	for (int i=0;i<vertexCount;i++)
		indices[i]=((i%2)==0)?(i/2):(vertexCount-1-i/2);
	glPushColor();
	glMultColor(color.r, color.g, color.b,0.5f);
	gtexture_get_engine()->getDefault(ShaderEngine::STDP_BASIC)->drawElements(ShaderProgram::TriangleStrip, vertexCount,ShaderProgram::DUSHORT, indices,true,NULL);
	glPopColor();
	delete[] indices;

	glPushColor();
	glMultColor(color.r, color.g, color.b,1);
	gtexture_get_engine()->getDefault(ShaderEngine::STDP_BASIC)->drawArrays(ShaderProgram::LineLoop, 0, vertexCount);
	glPopColor();
}

void b2DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
	const float32 k_segments = 16.0f;
    const int vertexCount=16;
	const float32 k_increment = 2.0f * b2_pi / k_segments;
	float32 theta = 0.0f;

	float				glVertices[vertexCount*2];
	for (int32 i = 0; i < k_segments; ++i)
	{
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		glVertices[i*2]=v.x;
		glVertices[i*2+1]=v.y;
		theta += k_increment;
	}

	glPushColor();
	glMultColor(color.r, color.g, color.b,1);
	gtexture_get_engine()->getDefault(ShaderEngine::STDP_BASIC)->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT,2, glVertices, vertexCount, true, NULL);
	gtexture_get_engine()->getDefault(ShaderEngine::STDP_BASIC)->drawArrays(ShaderProgram::LineLoop, 0, vertexCount);
	glPopColor();
}

void b2DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
	const float32 k_segments = 16.0f;
    const int vertexCount=16;
	const float32 k_increment = 2.0f * b2_pi / k_segments;
	float32 theta = 0.0f;

	float				glVertices[vertexCount*2];
	for (int32 i = 0; i < k_segments; ++i)
	{
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		glVertices[i*2]=v.x;
		glVertices[i*2+1]=v.y;
		theta += k_increment;
	}


	uint16_t *indices=new uint16_t[vertexCount];
	for (int i=0;i<vertexCount;i++)
		indices[i]=((i%2)==0)?(i/2):(vertexCount-1-i/2);
	delete[] indices;

	glPushColor();
	glMultColor(color.r, color.g, color.b,0.5f);
	gtexture_get_engine()->getDefault(ShaderEngine::STDP_BASIC)->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2, glVertices, vertexCount, true, NULL);
	gtexture_get_engine()->getDefault(ShaderEngine::STDP_BASIC)->drawElements(ShaderProgram::TriangleStrip, vertexCount,ShaderProgram::DUSHORT, indices,true,NULL);
	glPopColor();
	glPushColor();
	glMultColor(color.r, color.g, color.b,1);
	gtexture_get_engine()->getDefault(ShaderEngine::STDP_BASIC)->drawArrays(ShaderProgram::LineLoop, 0, vertexCount);
	glPopColor();

	delete[] indices;

	// Draw the axis line
	DrawSegment(center,center+radius*axis,color);
}

void b2DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	glPushColor();
	glMultColor(color.r, color.g, color.b,1);
	float				glVertices[] = {
		p1.x,p1.y,p2.x,p2.y
	};
	gtexture_get_engine()->getDefault(ShaderEngine::STDP_BASIC)->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2, glVertices, 2, true, NULL);
	gtexture_get_engine()->getDefault(ShaderEngine::STDP_BASIC)->drawArrays(ShaderProgram::Lines, 0, 2);
	glPopColor();
}

void b2DebugDraw::DrawTransform(const b2Transform& xf)
{
	b2Vec2 p1 = xf.p, p2;
	const float32 k_axisScale = 0.4f;

	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	DrawSegment(p1,p2,b2Color(1,0,0));

	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	DrawSegment(p1,p2,b2Color(0,1,0));
}

void b2DebugDraw::doDraw(const CurrentTransform& , float _UNUSED(sx), float _UNUSED(sy), float _UNUSED(ex), float _UNUSED(ey))
{
	if (world_)
	{

		float physicsScale = application_->getPhysicsScale();

		Matrix4 modelMat=gtexture_get_engine()->getModel();
		float mc[16];
		memcpy(mc,modelMat.get(),16*sizeof(float));
		mc[0]*=physicsScale;
		mc[5]*=physicsScale;
		Matrix4 scaledMat;
		scaledMat.set(mc);
		gtexture_get_engine()->setModel(scaledMat);

		world_->DrawDebugData();

		gtexture_get_engine()->setModel(modelMat);
	}
}

int Box2DBinder2::b2DebugDraw_create(lua_State* L)
{
	Binder binder(L);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	b2DebugDraw* debugDraw = new b2DebugDraw(application);
	binder.pushInstance("b2DebugDraw", debugDraw->proxy_);

	return 1;
}

int Box2DBinder2::b2DebugDraw_destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	b2DebugDraw* debugDraw = static_cast<b2DebugDraw*>(static_cast<SpriteProxy *>(ptr)->getContext());
	debugDraw->proxy_->unref();

	return 0;
}

int Box2DBinder2::b2World_setDebugDraw(lua_State* L)
{
    //StackChecker checker(L, "b2World_setDebugDraw", 0);

	Binder binder(L);
	b2WorldED* world = static_cast<b2WorldED*>(binder.getInstance("b2World", 1));

	if (lua_isnoneornil(L, 2))
	{
		world->SetDebugDraw(NULL);
	}
	else
	{
		b2DebugDraw* debugDraw = static_cast<b2DebugDraw*>(static_cast<SpriteProxy *>(binder.getInstance("b2DebugDraw", 2))->getContext());
		world->SetDebugDraw(debugDraw);
	}

	return 0;
}


int Box2DBinder2::b2DebugDraw_setFlags(lua_State* L)
{
	Binder binder(L);
	b2DebugDraw* debugDraw = static_cast<b2DebugDraw*>(static_cast<SpriteProxy *>(binder.getInstance("b2DebugDraw", 1))->getContext());

	debugDraw->SetFlags(luaL_checkinteger(L, 2));

	return 0;
}

int Box2DBinder2::b2DebugDraw_getFlags(lua_State* L)
{
	Binder binder(L);
	b2DebugDraw* debugDraw = static_cast<b2DebugDraw*>(static_cast<SpriteProxy *>(binder.getInstance("b2DebugDraw", 1))->getContext());

	lua_pushinteger(L, debugDraw->GetFlags());

	return 1;
}

int Box2DBinder2::b2DebugDraw_appendFlags(lua_State* L)
{
	Binder binder(L);
	b2DebugDraw* debugDraw = static_cast<b2DebugDraw*>(static_cast<SpriteProxy *>(binder.getInstance("b2DebugDraw", 1))->getContext());

	debugDraw->AppendFlags(luaL_checkinteger(L, 2));

	return 0;
}

int Box2DBinder2::b2DebugDraw_clearFlags(lua_State* L)
{
	Binder binder(L);
	b2DebugDraw* debugDraw = static_cast<b2DebugDraw*>(static_cast<SpriteProxy *>(binder.getInstance("b2DebugDraw", 1))->getContext());

	debugDraw->ClearFlags(luaL_checkinteger(L, 2));

	return 0;
}

static void b2PSS_Draw(void *c,const CurrentTransform&t, float sx, float sy, float ex, float ey)
{
	((b2ParticleSystemSprite *)c)->doDraw(t,sx, sy, ex, ey);
}

static void b2PSS_Destroy(void *c)
{
	delete ((b2ParticleSystemSprite *)c);
}

b2ParticleSystemSprite::b2ParticleSystemSprite(LuaApplication* application,b2ParticleSystem* b2ps,b2WorldED *world)
{
	ps_=b2ps;
	world_=world;
	application_=application;
	texturebase_=NULL;
	psize_=0;
	proxy_=gtexture_get_spritefactory()->createProxy(application->getApplication(), this, b2PSS_Draw, b2PSS_Destroy);
}

void b2ParticleSystemSprite::SetTexture(TextureBase *texturebase,float psize)
{
	TextureBase *originaltexturebase = texturebase_;
	if (texturebase)
	{
		texturebase_ = texturebase;
		texturebase_->ref();
	}
	if (originaltexturebase)
		originaltexturebase->unref();
	psize_=psize;
}

b2ParticleSystemSprite::~b2ParticleSystemSprite()
{
	if (texturebase_ != NULL)
		texturebase_->unref();
	//TODO Destroy system ?
}

void b2ParticleSystemSprite::doDraw(const CurrentTransform& , float _UNUSED(sx), float _UNUSED(sy), float _UNUSED(ex), float _UNUSED(ey))
{
	if (ps_)
	{
		float physicsScale = application_->getPhysicsScale();

		Matrix4 modelMat=gtexture_get_engine()->getModel();
		float mc[16];
		memcpy(mc,modelMat.get(),16*sizeof(float));
		mc[0]*=physicsScale;
		mc[5]*=physicsScale;
		Matrix4 scaledMat;
		scaledMat.set(mc);
		gtexture_get_engine()->setModel(scaledMat);

		ShaderProgram *p=proxy_->getShader();
		if (!p)
            p=gtexture_get_engine()->getDefault(ShaderEngine::STDP_PARTICLE,texturebase_?ShaderEngine::STDPV_TEXTURED:0);
		if (p)
		{
			int pc=ps_->GetParticleCount();
			p->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT, 2,ps_->GetPositionBuffer(), pc, true, NULL);
			p->setData(ShaderProgram::DataColor, ShaderProgram::DUBYTE, 4,ps_->GetColorBuffer(), pc, true, NULL);

			float textureInfo[4]={0,0,0,0};
			if (texturebase_)
			{
				gtexture_get_engine()->bindTexture(0,gtexture_getInternalTexture(texturebase_->data->gid));
				textureInfo[0]=(float)texturebase_->data->width / (float)texturebase_->data->exwidth;
				textureInfo[1]=(float)texturebase_->data->height / (float)texturebase_->data->exheight;
				textureInfo[2]=1.0/texturebase_->data->exwidth;
				textureInfo[3]=1.0/texturebase_->data->exheight;
			}
			int sc=p->getSystemConstant(ShaderProgram::SysConst_TextureInfo);
			if (sc>=0)
				p->setConstant(sc,ShaderProgram::CFLOAT4,1,textureInfo);
			sc = p->getSystemConstant(ShaderProgram::SysConst_ParticleSize);
			if (sc>=0)
			{
				float rad=((psize_==0)?ps_->GetRadius():psize_)*2; //Point Size is width, e.q diameter
				p->setConstant(sc,ShaderProgram::CFLOAT,1,&rad);
			}
			p->drawArrays(ShaderProgram::Point, 0, pc);
		}

		gtexture_get_engine()->setModel(modelMat);
	}
}
