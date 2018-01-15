/*
 * lqSprites.h
 *
 *  Created on: 22 oct. 2017
 *      Author: Nicolas
 */

#ifndef LQSPRITES_H_
#define LQSPRITES_H_
#include "lqWorld.h"
#include "liquidfunbinder.h"
#include <stack>
#include <Box2D/Box2D.h>
#include <ogl.h>
#include <color.h>
#include <sprite.h>
#include <texturebase.h>
#include "luaapplication.h"

class b2DebugDraw : public b2Draw
{
public:
    b2DebugDraw(LuaApplication* application);

	virtual ~b2DebugDraw();
	SpriteProxy *proxy_;

    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
private:
	struct Color
	{
		Color() {}
		Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

		float r, g, b, a;
	};

	std::stack<Color> colorStack;

	void glPushColor()
	{
		Color c;
		gtexture_get_engine()->getColor(c.r,c.g,c.b,c.a);
		colorStack.push(c);
	}

	void glPopColor()
	{
		Color c = colorStack.top();
		colorStack.pop();

		gtexture_get_engine()->setColor(c.r, c.g, c.b, c.a);
	}

	void glMultColor(float r, float g, float b, float a)
	{
		Color c = colorStack.top();
		gtexture_get_engine()->setColor(c.r*r*a, c.g*g*a, c.b*b*a, c.a*a);
	}

private:
	virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);
	virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);
	virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
	virtual void DrawTransform(const b2Transform& xf);
    virtual void DrawParticles(const b2Vec2 * _UNUSED(centers), float32 _UNUSED(radius), const b2ParticleColor *_UNUSED(colors), int32 _UNUSED(count)) { }

private:
	friend class b2WorldED;
	b2WorldED* world_;
	LuaApplication* application_;
};

class b2ParticleSystemSprite
{
public:
    b2ParticleSystemSprite(LuaApplication* application,b2ParticleSystem* b2ps,b2WorldED *world);
	virtual ~b2ParticleSystemSprite();
	b2ParticleSystem* GetSystem() { return ps_; }
	b2WorldED* GetWorld() { return world_; }
	void SetTexture(TextureBase *texture,float psize);
	SpriteProxy *proxy_;
    void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
private:
	LuaApplication* application_;
	b2ParticleSystem* ps_;
	b2WorldED *world_;
	TextureBase* texturebase_;
	float psize_;
};


#endif /* LQSPRITES_H_ */
