#ifndef SPRITE_H
#define SPRITE_H

#include "eventdispatcher.h"
#include "transform.h"
//#include "graphics.h"
#include "colortransform.h"
#include "gstatus.h"
#include "graphicsbase.h"
#include "matrix.h"
#include <list>
#include <vector>
#include <string>
#include <map>

typedef Matrix4 CurrentTransform;
typedef Matrix4 Matrix;

class Application;
class Stage;

class Sprite : public EventDispatcher
{
public:
    Sprite(Application* application);
	virtual ~Sprite();

    void draw(const CurrentTransform&, float sx, float sy, float ex, float ey);

	void addChild(Sprite* sprite, GStatus* status = 0);
	void removeChild(Sprite* child, GStatus* status = 0);
	void removeChild(int index, GStatus* status = 0);
	bool contains(Sprite* sprite) const;
	void replaceChild(Sprite* oldChild, Sprite* newChild);
	bool canChildBeAdded(Sprite* sprite, GStatus* status = 0);
	void addChildAt(Sprite* sprite, int index, GStatus* status = 0);
	bool canChildBeAddedAt(Sprite* sprite, int index, GStatus* status = 0);
	int getChildIndex(Sprite* sprite, GStatus* status = 0);
	void setChildIndex(Sprite* child, int index, GStatus* status = 0);
	void swapChildren(Sprite* child1, Sprite* child2, GStatus* status = 0);
	void swapChildrenAt(int index1, int index2, GStatus* status = 0);
	void removeChildAt(int index, GStatus* status = 0);
	
	void localToGlobal(float x, float y, float* tx, float* ty) const;
	void globalToLocal(float x, float y, float* tx, float* ty) const;
	void localToGlobal(float x, float y, float z, float* tx, float* ty, float* tz) const;
	void globalToLocal(float x, float y, float z, float* tx, float* ty, float* tz) const;

	void setAlpha(float alpha);
	float alpha() const;
	
	int childCount() const
	{
		return (int)children_.size();
	}

	Sprite* child(int index) const
	{
		return children_[index];
	}

	Sprite* getChildAt(int index, GStatus* status = 0) const;

	Sprite* parent() const
	{
		return parent_;
	}

	const ColorTransform& colorTransform() const
	{
		if (colorTransform_ == 0)
			colorTransform_ = new ColorTransform();
		
		return *colorTransform_;
	}

	void setColorTransform(const ColorTransform& colorTransform);

    void setRedMultiplier(float redMultiplier);
    void setGreenMultiplier(float greenMultiplier);
    void setBlueMultiplier(float blueMultiplier);
    void setAlphaMultiplier(float alphaMultiplier);

    float getRedMultiplier() const;
    float getGreenMultiplier() const;
    float getBlueMultiplier() const;
    float getAlphaMultiplier() const;

    //Set clipping rectangle, cw or ch negative to disable
    void setClip(float cx,float cy,float cw,float ch)
    {
    	clipx_=cx;
    	clipy_=cy;
    	clipw_=cw;
    	cliph_=ch;
    }

    float clipX() const
    {
        return clipx_;
    }

    float clipY() const
    {
        return clipy_;
    }

    float clipW() const
    {
        return clipw_;
    }
    float clipH() const

    {
        return cliph_;
    }

	void setRotation(float r)
	{
        localTransform_.setRotationZ(r);
	}

	void setRotationX(float r)
	{
        localTransform_.setRotationX(r);
	}

	void setRotationY(float r)
	{
        localTransform_.setRotationY(r);
	}

	void setScaleX(float sx)
	{
        localTransform_.setScaleX(sx);
	}

	void setScaleY(float sy)
	{
        localTransform_.setScaleY(sy);
	}

	void setScaleZ(float sz)
	{
        localTransform_.setScaleZ(sz);
	}

	void setScaleXY(float sx, float sy)
	{
        localTransform_.setScaleXY(sx, sy);
	}

	void setScaleXYZ(float sx, float sy, float sz)
	{
        localTransform_.setScaleXYZ(sx, sy, sz);
	}

	void setScale(float s)
	{
        localTransform_.setScaleXYZ(s, s, s);
	}

    void setSkewX(float sx)
    {
        localTransform_.setSkewX(sx);
    }

    void setSkewY(float sy)
    {
        localTransform_.setSkewY(sy);
    }

    void setSkewXY(float sx, float sy)
    {
        localTransform_.setSkewXY(sx, sy);
    }

	void setX(float x)
	{
        localTransform_.setX(x);
	}

	void setY(float y)
	{
        localTransform_.setY(y);
	}

	void setZ(float z)
	{
        localTransform_.setZ(z);
	}

	void setXY(float x, float y)
	{
        localTransform_.setXY(x, y);
	}

	void setXYZ(float x, float y, float z)
	{
        localTransform_.setXYZ(x, y, z);
	}

    void setRefX(float x)
    {
        localTransform_.setRefX(x);
    }

    void setRefY(float y)
    {
        localTransform_.setRefY(y);
    }

    void setRefZ(float z)
    {
        localTransform_.setRefZ(z);
    }

    void setRefXY(float x, float y)
    {
        localTransform_.setRefXY(x, y);
    }

    void setRefXYZ(float x, float y, float z)
    {
        localTransform_.setRefXYZ(x, y, z);
    }

	float rotation() const
	{
        return localTransform_.rotationZ();
	}

	float rotationX() const
	{
        return localTransform_.rotationX();
	}

	float rotationY() const
	{
        return localTransform_.rotationY();
	}

	float scaleX() const
	{
        return localTransform_.scaleX();
	}

	float scaleY() const
	{
        return localTransform_.scaleY();
	}

	float scaleZ() const
	{
        return localTransform_.scaleZ();
	}

    float skewX() const
    {
        return localTransform_.skewX();
    }

    float skewY() const
    {
        return localTransform_.skewY();
    }

	float x() const
	{
        return localTransform_.x();
	}

	float y() const
	{
        return localTransform_.y();
	}

	float z() const
	{
        return localTransform_.z();
	}

	float refX() const
    {
        return localTransform_.refX();
    }

    float refY() const
    {
        return localTransform_.refY();
    }

    float refZ() const
    {
        return localTransform_.refZ();
    }

    void setMatrix(float m11, float m12, float m21, float m22, float tx, float ty)
	{
        localTransform_.setMatrix(m11, m12, m21, m22, tx, ty);
	}

	void setMatrix(const Transform *matrix)
	{
        localTransform_=*matrix;
	}

	const Matrix4& matrix() const
	{
        return localTransform_.matrix();
	}

	// Gets the bounds of Sprite in its own coordinate system
	void objectBounds(float* minx, float* miny, float* maxx, float* maxy,bool visible=false) const;

	// Gets the bounds of Sprite after transformed by its Matrix.
	// localBounds = Matrix * objectBounds
	void localBounds(float* minx, float* miny, float* maxx, float* maxy,bool visible=false) const;
	
#if 0
	// Gets the bounds of Sprite after transformed by localToGlobal
	void globalBounds(float* minx, float* miny, float* maxx, float* maxy) const;
#endif

	// Indicates the width of the sprite, in pixels.
	// The width is calculated based on the local bounds of the content of the sprite.
	float width() const;

	// Indicates the height of the sprite, in pixels.
	// The height is calculated based on the local bounds of the content of the sprite.
	float height() const;

	// Gets whether or not the sprite is visible.
	bool visible() const
	{
		return isVisible_;
	}

	// Sets whether or not the sprite is visible.
	void setVisible(bool visible)
	{
		isVisible_ = visible;
	}

	// Evaluates the sprite to see if its bounds overlaps or intersects with the point specified by the x and y parameters.
	// The x and y parameters specify a point in the global coordinate space.
	bool hitTestPoint(float x, float y, bool visible=false) const;
	
	virtual bool isStage() const
	{
		return false;
	}

	static const std::set<Sprite*>& allSprites()
	{
		return allSprites_;
	}

    void getBounds(const Sprite* targetCoordinateSpace, float* minx, float* miny, float* maxx, float* maxy) const;

	void clearBlendFunc();
	void setBlendFunc(ShaderEngine::BlendFactor sfactor, ShaderEngine::BlendFactor dfactor);

	struct ShaderParam
	{
		std::string name;
		ShaderProgram::ConstantType type;
		int mult;
		std::vector<float> data;
	};
	void setShader(ShaderProgram *shader);
	void setShaderConstant(ShaderParam p)
	{
		shaderParams_[p.name]=p;
	}

	void set(const char* param, float value, GStatus* status = NULL);
	float get(const char* param, GStatus* status = NULL);
	void set(int param, float value, GStatus* status = NULL);
	float get(int param, GStatus* status = NULL);

	const Transform& transform() const
	{
        return localTransform_;
	}

    void setTransform(const Transform& transform)
	{
        localTransform_ = transform;
	}



protected:
	virtual void extraBounds(float* minx, float* miny, float* maxx, float* maxy) const
	{
		if (minx)
			*minx =  1e30f;
		if (miny)
			*miny =  1e30f;
		if (maxx)
			*maxx = -1e30f;
		if (maxy)
			*maxy = -1e30f;
	}

protected:
	void recursiveDispatchEvent(Event* event, bool canBeStopped, bool reverse);

private:
    Stage *getStage() const;
    void boundsHelper(const Matrix4& transform, float* minx, float* miny, float* maxx, float* maxy,std::stack<Matrix> parentXform,bool visible=false) const;

protected:
    Application *application_;

private:
	bool isVisible_;
    Transform localTransform_;
    mutable Matrix4 worldTransform_;
//	Graphics* graphics_;

	ShaderEngine::BlendFactor sfactor_, dfactor_;

	typedef std::vector<Sprite*> SpriteVector;
	SpriteVector children_;
	Sprite* parent_;

	mutable ColorTransform* colorTransform_;
	float alpha_;
	float clipx_,clipy_,clipw_,cliph_;

private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
    virtual void childrenDrawn();

protected:
	static std::set<Sprite*> allSprites_;
	static std::set<Sprite*> allSpritesWithListeners_;

protected:
	ShaderProgram *shader_;
	std::map<std::string,ShaderParam> shaderParams_;
//	typedef std::list<GraphicsBase, Gideros::STLAllocator<GraphicsBase, StdAllocator> > GraphicsBaseList;
//	GraphicsBaseList graphicsBases_;

protected:
	virtual void eventListenersChanged();
};

#endif

