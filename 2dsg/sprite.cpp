#include "sprite.h"
#include "ogl.h"
#include <algorithm>
#include <cassert>
#include <stack>
#include "color.h"
#include "blendfunc.h"
#include "stage.h"
#include <application.h>
#include "layoutevent.h"
#include "bitmap.h"

std::set<Sprite*> Sprite::allSprites_;
std::set<Sprite*> Sprite::allSpritesWithListeners_;

Sprite::Sprite(Application* application) :
        spriteWithLayoutCount(0), application_(application), isVisible_(true), parent_(NULL), reqWidth_(0), reqHeight_(0), drawCount_(0) {
	allSprites_.insert(this);

//	graphicsBases_.push_back(GraphicsBase());
	stopPropagationMask_=0;

	alpha_ = 1;
	colorTransform_ = 0;
//	graphics_ = 0;

	sfactor_ = (ShaderEngine::BlendFactor) -1;
	dfactor_ = (ShaderEngine::BlendFactor) -1;

	clipy_ = -1;
	clipx_ = -1;
	clipw_ = -1;
	cliph_ = -1;

	stencil_.dTest=false;
	layoutState=NULL;
	layoutConstraints=NULL;
}

Sprite::~Sprite() {
	delete colorTransform_;
//	delete graphics_;

	for (std::size_t i = 0; i < children_.size(); ++i){
		children_[i]->parent_ = 0;
		children_[i]->unref();
	}
	
	allSprites_.erase(this);
	allSpritesWithListeners_.erase(this);

	std::map<int,struct _ShaderSpec>::iterator it=shaders_.begin();
	while (it!=shaders_.end())
	{
		if (it->second.shader)
			it->second.shader->Release();
		it++;
	}

	clearLayoutState();
	clearLayoutConstraints();
}

void Sprite::setupShader(struct _ShaderSpec &spec) {
	if (spec.shader) {
		int sc=spec.shader->getSystemConstant(ShaderProgram::SysConst_Bounds);
		if (sc>=0)
		{
			float bounds[4]={0,0,0,0};
			extraBounds(bounds+0,bounds+1,bounds+2,bounds+3);
			spec.shader->setConstant(sc,ShaderProgram::CFLOAT4,1,bounds);
		}
    }

	for(std::map<std::string,ShaderParam>::iterator it = spec.params.begin(); it != spec.params.end(); ++it) {
			ShaderParam *p=&(it->second);
			int idx=spec.shader->getConstantByName(p->name.c_str());
			if (idx>=0)
				spec.shader->setConstant(idx,p->type,p->mult,&(p->data[0]));
	}
}

void Sprite::setShader(ShaderProgram *shader,ShaderEngine::StandardProgram id,int variant,bool inherit) {
	int sid=(id<<8)|variant;
	std::map<int,struct _ShaderSpec>::iterator it=shaders_.find(sid);
	if (shader)
		shader->Retain();
	if (it!=shaders_.end()) {
		if (it->second.shader)
			it->second.shader->Release();
		if (shader) {
			it->second.shader=shader;
			it->second.inherit=inherit;
		}
		else {
			it->second.params.clear();
			shaders_.erase(it);
		}
	}
	else if (shader) {
		struct _ShaderSpec sp;
		sp.shader=shader;
		sp.inherit=inherit;
		shaders_[sid]=sp;
	}
}

bool Sprite::setShaderConstant(ShaderParam p,ShaderEngine::StandardProgram id,int variant)
{
	int sid=(id<<8)|variant;
	std::map<int,struct _ShaderSpec>::iterator it=shaders_.find(sid);
	if (it!=shaders_.end()) {
		it->second.params[p.name]=p;
		return true;
	}
	else
		return false;
}

ShaderProgram *Sprite::getShader(ShaderEngine::StandardProgram id,int variant)
{
	int sid=(id<<8)|variant;
	std::map<int,struct _ShaderSpec>::iterator it=shaders_.find(sid);
	if (it!=shaders_.end()) {
		setupShader(it->second);
		return it->second.shader;
	}
	it=shaders_.find(0);
	if (it!=shaders_.end()) {
		setupShader(it->second);
		return it->second.shader;
	}
	Sprite *p=parent();
	while (p) {
		it=p->shaders_.find(sid);
        if (it!=p->shaders_.end()) {
			if (it->second.inherit) {
				setupShader(it->second);
				return it->second.shader;
			}
		}
        p=p->parent();
    }
	return ShaderEngine::Engine->getDefault(id, variant);
}

void Sprite::doDraw(const CurrentTransform&, float sx, float sy, float ex,
		float ey) {
 G_UNUSED(sx); G_UNUSED(sy); G_UNUSED(ex); G_UNUSED(ey);
}

void setupEffectShader(Bitmap *source,Sprite::Effect &e)
{
	source->setShader(e.shader);
    if (e.shader)
	for(std::map<std::string,Sprite::ShaderParam>::iterator it = e.params.begin(); it != e.params.end(); ++it) {
			Sprite::ShaderParam *p=&(it->second);
			int idx=e.shader->getConstantByName(p->name.c_str());
			if (idx>=0)
				e.shader->setConstant(idx,p->type,p->mult,&(p->data[0]));
	}
    for (size_t t=0;t<e.textures.size();t++)
        if (e.textures[t]) {
            if (t==0)
                source->setTexture(e.textures[t]);
            else
                ShaderEngine::Engine->bindTexture(t,e.textures[t]->data->id());
        }
}

void Sprite::setEffectStack(std::vector<Effect> effects,EffectUpdateMode mode) {
	int diff=0;
	if (effectStack_.size()>0) {
		diff-=1;
		for (size_t i=0;i<effectStack_.size();i++) {
			if (effectStack_[i].shader)
				effectStack_[i].shader->Release();
			if (effectStack_[i].buffer)
				effectStack_[i].buffer->unref();
            for (size_t t=0;t<effectStack_[i].textures.size();t++)
                if (effectStack_[i].textures[t])
                    effectStack_[i].textures[t]->unref();
		}
	}
	effectStack_=effects;
	if (effectStack_.size()>0) {
		diff+=1;
		for (size_t i=0;i<effectStack_.size();i++) {
			if (effectStack_[i].shader)
				effectStack_[i].shader->Retain();
			if (effectStack_[i].buffer)
				effectStack_[i].buffer->ref();
            for (size_t t=0;t<effectStack_[i].textures.size();t++)
                if (effectStack_[i].textures[t])
                    effectStack_[i].textures[t]->ref();
		}
	}
	effectsDirty_=true;
	effectsMode_=mode;

	Sprite *p=this;
	while (p) {
		p->spriteWithLayoutCount+=diff;
        p=p->parent();
	}
}

bool Sprite::setEffectShaderConstant(size_t effectNumber,ShaderParam p)
{
	if (effectNumber>=effectStack_.size())
		return false;
	effectStack_[effectNumber].params[p.name]=p;
	return true;
}

void Sprite::updateEffects()
{
	if (effectsMode_!=CONTINUOUS) {
		if (!effectsDirty_) return;
	}
	effectsDrawing_=true;
    float swidth,sheight;
    if (effectStack_.size()) {
        float minx, miny, maxx, maxy;

        localBounds(&minx, &miny, &maxx, &maxy);

        if (minx > maxx || miny > maxy)
               return; //Empty Sprite, do nothing
        swidth=maxx - minx;
        sheight=maxy - miny;
        swidth*=application_->getLogicalScaleX();
        sheight*=application_->getLogicalScaleY();
    }
	for (size_t i=0;i<effectStack_.size();i++) {
		if (effectStack_[i].buffer) {
			if (i==0) { //First stage, draw the Sprite normally onto the first buffer
                if (effectStack_[i].autoBuffer) {
                    float minx, miny, maxx, maxy;

                    effectStack_[i].transform.transformPoint(0,0,&minx,&miny);
                    maxx=minx; maxy=miny;
                    float tx,ty;
                    effectStack_[i].transform.transformPoint(swidth,0,&tx,&ty);
                    maxx=std::max(maxx,tx); maxy=std::max(maxy,ty);
                    minx=std::min(minx,tx); miny=std::min(miny,ty);
                    effectStack_[i].transform.transformPoint(0,sheight,&tx,&ty);
                    maxx=std::max(maxx,tx); maxy=std::max(maxy,ty);
                    minx=std::min(minx,tx); miny=std::min(miny,ty);
                    effectStack_[i].transform.transformPoint(swidth,sheight,&tx,&ty);
                    maxx=std::max(maxx,tx); maxy=std::max(maxy,ty);
                    minx=std::min(minx,tx); miny=std::min(miny,ty);
                    maxx=std::max(maxx,.0F); maxy=std::max(maxy,.0F);
                    minx=std::max(minx,.0F); miny=std::max(miny,.0F);
                    swidth=maxx-minx; sheight=maxy-miny;
                    effectStack_[i].buffer->resize(ceilf(swidth),ceilf(sheight));
                }
                if (effectStack_[i].clearBuffer)
                    effectStack_[i].buffer->clear(0,0,0,0,-1,-1);
                Matrix invL=localTransform_.matrix().inverse();
                invL.scale(application_->getLogicalScaleX(),application_->getLogicalScaleY(),1);
                invL*=effectStack_[i].transform;

                effectStack_[i].buffer->draw(this,invL);
			}
			else if (effectStack_[i-1].buffer) {
                if (effectStack_[i].autoBuffer) {
                    float minx, miny, maxx, maxy;

                    effectStack_[i].transform.transformPoint(0,0,&minx,&miny);
                    maxx=minx; maxy=miny;
                    float tx,ty;
                    effectStack_[i].transform.transformPoint(swidth,0,&tx,&ty);
                    maxx=std::max(maxx,tx); maxy=std::max(maxy,ty);
                    minx=std::min(minx,tx); miny=std::min(miny,ty);
                    effectStack_[i].transform.transformPoint(0,sheight,&tx,&ty);
                    maxx=std::max(maxx,tx); maxy=std::max(maxy,ty);
                    minx=std::min(minx,tx); miny=std::min(miny,ty);
                    effectStack_[i].transform.transformPoint(swidth,sheight,&tx,&ty);
                    maxx=std::max(maxx,tx); maxy=std::max(maxy,ty);
                    minx=std::min(minx,tx); miny=std::min(miny,ty);
                    maxx=std::max(maxx,.0F); maxy=std::max(maxy,.0F);
                    minx=std::max(minx,.0F); miny=std::max(miny,.0F);
                    swidth=maxx-minx; sheight=maxy-miny;
                    effectStack_[i].buffer->resize(ceilf(swidth),ceilf(sheight));
                }
                if (effectStack_[i].clearBuffer)
                    effectStack_[i].buffer->clear(0,0,0,0,-1,-1);
				Bitmap source(application_,effectStack_[i-1].buffer);
				setupEffectShader(&source,effectStack_[i-1]);
				effectStack_[i].buffer->draw(&source,effectStack_[i].transform);
			}
		}
	}
	effectsDirty_=false;
	effectsDrawing_=false;
}

void Sprite::redrawEffects() {
	Sprite *p=this;
	while (p) {
		p->effectsDirty_=true;
		p=p->parent();
	}
}

GridBagLayout *Sprite::getLayoutState()
{
	if (!layoutState) {
		layoutState=new GridBagLayout();
		Sprite *p=this;
		while (p) {
			p->spriteWithLayoutCount++;
			p=p->parent();
		}
	}
	return layoutState;
}

void Sprite::clearLayoutState() {
	if (layoutState) {
		delete layoutState;
		Sprite *p=this;
		while (p) {
			p->spriteWithLayoutCount--;
            p=p->parent();
		}
	}
	layoutState=NULL;
}

GridBagConstraints *Sprite::getLayoutConstraints()
{
	if (!layoutConstraints)
		layoutConstraints=new GridBagConstraints();
    if (parent_&&parent_->layoutState)
        parent_->layoutState->dirty=true;
    return layoutConstraints;
}

void Sprite::clearLayoutConstraints()
{
	if (layoutConstraints)
		delete layoutConstraints;
	layoutConstraints=NULL;
}

void Sprite::layoutSizesChanged() {
    if (layoutConstraints) {
		if ((layoutConstraints->prefWidth==-1)
				||(layoutConstraints->aminWidth==-1)
				||(layoutConstraints->prefHeight==-1)
				||(layoutConstraints->aminHeight==-1)
				) {
	        Sprite *p=parent_;
            while (p&&(p->layoutState)&&(!p->layoutState->optimizing))
	        {
	        	p->layoutState->dirty=true;
	        	p=p->parent_;
	        }
		}
	}
}


void Sprite::childrenDrawn() {

}

/*
 void Sprite::draw()
 {
 if (isVisible_ == false)
 return;

 if (isWhite_ == false)
 {
 glPushColor();
 glMultColor(r_, g_, b_, a_);
 }

 if (transform_ != 0 && transform_->matrix().type() != Matrix::eIdentity)
 {
 glPushMatrix();

 switch (transform_->matrix().type())
 {
 case Matrix::eTranslationOnly:
 glTranslatef(transform_->matrix().tx(), transform_->matrix().ty(), 0);
 break;
 default:
 glMultMatrixf(transform_->matrix().data());
 break;
 }
 }

 if (sfactor_ != -1)
 {
 glPushBlendFunc();
 glSetBlendFunc(sfactor_, dfactor_);
 }

 //	if (graphics_ != 0)
 //		graphics_->draw();

 for (GraphicsBaseList::iterator iter = graphicsBases_.begin(), e = graphicsBases_.end(); iter != e; ++iter)
 iter->draw();

 for (std::size_t i = 0; i < children_.size(); ++i)
 children_[i]->draw();

 if (sfactor_ != -1)
 glPopBlendFunc();

 if (transform_ != 0 && transform_->matrix().type() != Matrix::eIdentity)
 glPopMatrix();

 if (isWhite_ == false)
 glPopColor();
 }

 */

template<typename T>
class GGPool {
public:
	~GGPool() {
		for (size_t i = 0; i < v_.size(); ++i)
			delete v_[i];
	}

	T *create() {
		T *result;

		if (v_.empty()) {
			result = new T;
		} else {
			result = v_.back();
			v_.pop_back();
		}

		return result;

	}

	void destroy(T *t) {
		v_.push_back(t);
	}

private:
	std::vector<T*> v_;
};

void Sprite::draw(const CurrentTransform& transform, float sx, float sy,
		float ex, float ey) {
	{
		this->worldTransform_ = transform * this->localTransform_.matrix();

		static GGPool<std::stack<Sprite*> > stackPool;
		std::stack<Sprite*> &stack = *stackPool.create();

        stack.push(this);

		while (!stack.empty()) {
			Sprite *sprite = stack.top();
			stack.pop();

			if (sprite->isVisible_ == false) {
				continue;
			}

            if ((sprite!=this)&&(sprite->parent_))
                sprite->worldTransform_ = sprite->parent_->worldTransform_
					* sprite->localTransform_.matrix();

			for (size_t i = 0; i < sprite->children_.size(); ++i)
				stack.push(sprite->children_[i]);
		}

		stackPool.destroy(&stack);
	}

	static GGPool<std::stack<std::pair<Sprite*, bool> > > stackPool;
	std::stack<std::pair<Sprite*, bool> > &stack = *stackPool.create();

	stack.push(std::make_pair(this, false));

	while (stack.empty() == false) {
		Sprite* sprite = stack.top().first;
		bool pop = stack.top().second;
		stack.pop();

		bool lastEffect=((!sprite->effectsDrawing_)&&(sprite->effectStack_.size()>0));

		if (pop == true) {
            sprite->drawCount_=1;
            for (size_t i = 0;i< sprite->children_.size();i++)
                sprite->drawCount_+=sprite->children_[i]->drawCount_;
            sprite->childrenDrawn();
			if (sprite->colorTransform_ != 0 || sprite->alpha_ != 1)
				glPopColor();
			if (sprite->sfactor_ != (ShaderEngine::BlendFactor)-1)
				glPopBlendFunc();
			if ((sprite->cliph_ >= 0) && (sprite->clipw_ >= 0))
				ShaderEngine::Engine->popClip();
			if (sprite->stencil_.dTest)
				ShaderEngine::Engine->popDepthStencil();
			continue;
		}

        sprite->drawCount_=0;

		if (sprite->isVisible_ == false) {
			continue;
		}

        ShaderEngine::Engine->setModel(sprite->worldTransform_);

		if (sprite->colorTransform_ != 0 || sprite->alpha_ != 1) {
			glPushColor();

			float r = 1, g = 1, b = 1, a = 1;

			if (sprite->colorTransform_) {
				r = sprite->colorTransform_->redMultiplier();
				g = sprite->colorTransform_->greenMultiplier();
				b = sprite->colorTransform_->blueMultiplier();
				a = sprite->colorTransform_->alphaMultiplier();
			}

            if (!lastEffect)
                glMultColor(r, g, b, a * sprite->alpha_);
		}

		if (sprite->sfactor_ != (ShaderEngine::BlendFactor)-1) {
			glPushBlendFunc();
			if (!lastEffect)
				glSetBlendFunc(sprite->sfactor_, sprite->dfactor_);
		}

		if ((sprite->cliph_ >= 0) && (sprite->clipw_ >= 0))
			ShaderEngine::Engine->pushClip(sprite->clipx_, sprite->clipy_,
					sprite->clipw_, sprite->cliph_);

		if (sprite->stencil_.dTest)
		{
			ShaderEngine::DepthStencil stencil =
				ShaderEngine::Engine->pushDepthStencil();
			stencil.sClear=sprite->stencil_.sClear;
			stencil.sFail=sprite->stencil_.sFail;
			stencil.dFail=sprite->stencil_.dFail;
			stencil.dPass=sprite->stencil_.dPass;
			stencil.sFunc=sprite->stencil_.sFunc;
			stencil.sMask=sprite->stencil_.sMask;
			stencil.sWMask=sprite->stencil_.sWMask;
			stencil.sRef=sprite->stencil_.sRef;
			if (!lastEffect)
				ShaderEngine::Engine->setDepthStencil(stencil);
		}

		if (lastEffect)
		{
            size_t i=sprite->effectStack_.size()-1;
			Bitmap source(application_,sprite->effectStack_[i].buffer);
			setupEffectShader(&source,sprite->effectStack_[i]);
            Matrix4 xform;
            if (sprite->parent_)
                xform=sprite->parent_->worldTransform_;
            else
                xform=transform;
            xform=xform*sprite->effectStack_[i].postTransform;
            xform=xform*sprite->localTransform_.matrix();
            if (sprite->effectStack_[0].autoBuffer) {
                Matrix mscale;
                mscale.scale(1/application_->getLogicalScaleX(),1/application_->getLogicalScaleY(),1);
                xform=xform*mscale;
            }
            ShaderEngine::Engine->setModel(xform);
            ((Sprite *)&source)->doDraw(xform, sx, sy, ex, ey);
		}
		else
			sprite->doDraw(sprite->worldTransform_, sx, sy, ex, ey);

		stack.push(std::make_pair(sprite, true));

		if (!lastEffect) //Don't draw subs if rendering last effect
		for (int i = (int) sprite->children_.size() - 1; i >= 0; --i)
			stack.push(std::make_pair(sprite->children_[i], false));
	}

	stackPool.destroy(&stack);
}

void Sprite::computeLayout() {
	if (!spriteWithLayoutCount) return;
	static GGPool<std::stack<Sprite*> > stackPool;
	std::stack<Sprite*> &stack = *stackPool.create();

	stack.push(this);

	while (!stack.empty()) {
		Sprite *sprite = stack.top();
		stack.pop();

		if ((sprite->isVisible_ == false)||(!(sprite->spriteWithLayoutCount))) {
			continue;
		}

		if (sprite->layoutState&&sprite->layoutState->dirty)
		{
			int loops=100; //Detect endless loops
			while(sprite->layoutState->dirty&&(loops--))
			{
				sprite->layoutState->dirty=false;
				float pwidth,pheight;
				sprite->getDimensions(pwidth, pheight);
				sprite->layoutState->ArrangeGrid(sprite,pwidth,pheight);
			}
			if (loops==0) //Gave up, mark as clean to prevent going through endless loop again
				sprite->layoutState->dirty=false;
		}

		for (size_t i = 0; i < sprite->children_.size(); ++i)
			stack.push(sprite->children_[i]);
		sprite->updateEffects();
	}

	stackPool.destroy(&stack);
}

bool Sprite::canChildBeAdded(Sprite* sprite, GStatus* status) {
	if (sprite == this) {
		if (status != 0)
			*status = GStatus(2024); // Error #2024: An object cannot be added as a child of itself.

		return false;
	}

	if (sprite->contains(this) == true) {
		if (status != 0)
			*status = GStatus(2150);// Error #2150: An object cannot be added as a child to one of it's children (or children's children, etc.).

		return false;
	}

	return true;
}

bool Sprite::canChildBeAddedAt(Sprite* sprite, int index, GStatus* status) {
	if (canChildBeAdded(sprite, status) == false)
		return false;

	if (index < 0 || index > childCount()) {
		if (status)
			*status = GStatus(2006); // Error #2006: The supplied index is out of bounds.
		return false;
	}

	return true;
}

void Sprite::addChild(Sprite* sprite, GStatus* status) {
	addChildAt(sprite, childCount(), status);
}

void Sprite::addChildAt(Sprite* sprite, int index, GStatus* status) {
	if (canChildBeAddedAt(sprite, index, status) == false)
		return;

	Stage* stage1 = sprite->getStage();

	if (stage1)
		stage1->setSpritesWithListenersDirty();

	if (sprite->parent_ == this) {
		*std::find(children_.begin(), children_.end(), sprite) = NULL;
		children_.insert(children_.begin() + index, sprite);
		children_.erase(
				std::find(children_.begin(), children_.end(), (Sprite*) NULL));
		return;
	}

	bool connected1 = stage1 != NULL;

	sprite->ref();		// guard

	if (sprite->parent_) {
		SpriteVector& children = sprite->parent_->children_;
		children.erase(std::find(children.begin(), children.end(), sprite));
		sprite->unref();
	}
	sprite->parent_ = this;

	children_.insert(children_.begin() + index, sprite);
	sprite->ref();
    if (layoutState&&sprite->layoutConstraints)
        layoutState->dirty=true;

    sprite->unref();	// unguard

	Stage *stage2 = sprite->getStage();

	if (stage2)
		stage2->setSpritesWithListenersDirty();

	Sprite *p=this;
	while (p) {
		p->spriteWithLayoutCount+=sprite->spriteWithLayoutCount;
		p=p->parent();
	}


	bool connected2 = stage2 != NULL;

	if (connected1 && !connected2) {
		Event event(Event::REMOVED_FROM_STAGE);
		sprite->recursiveDispatchEvent(&event, false, false);
	} else if (!connected1 && connected2) {
		Event event(Event::ADDED_TO_STAGE);
		sprite->recursiveDispatchEvent(&event, false, false);
	}
}

/**
 Returns the index position of a child Sprite instance.
 */
int Sprite::getChildIndex(Sprite* sprite, GStatus* status) {
	SpriteVector::iterator iter = std::find(children_.begin(), children_.end(),
			sprite);

	if (iter == children_.end()) {
		if (status)
			*status = GStatus(2025); // Error #2025: The supplied Sprite must be a child of the caller.
	}

	return iter - children_.begin();
}

/**
 Changes the position of an existing child in the display object container.
 */
void Sprite::setChildIndex(Sprite* child, int index, GStatus* status) {
	int currentIndex = getChildIndex(child, status);

	if (currentIndex == childCount())
		return;

	if (index < 0 || index > childCount()) {
		if (status)
			*status = GStatus(2006); // Error #2006: The supplied index is out of bounds.
		return;
	}

	children_.erase(children_.begin() + currentIndex);
	children_.insert(children_.begin() + index, child);
}

void Sprite::swapChildren(Sprite* child1, Sprite* child2, GStatus* status) {
	int index1 = getChildIndex(child1, status);
	if (index1 == childCount())
		return;

	int index2 = getChildIndex(child2, status);
	if (index2 == childCount())
		return;

	std::swap(children_[index1], children_[index2]);
}

void Sprite::swapChildrenAt(int index1, int index2, GStatus* status) {
	if (index1 < 0 || index1 >= childCount()) {
		if (status)
			*status = GStatus(2006); // Error #2006: The supplied index is out of bounds.
		return;
	}

	if (index2 < 0 || index2 >= childCount()) {
		if (status)
			*status = GStatus(2006); // Error #2006: The supplied index is out of bounds.
		return;
	}

	std::swap(children_[index1], children_[index2]);
}

Sprite* Sprite::getChildAt(int index, GStatus* status) const {
	if (index < 0 || index >= childCount()) {
		if (status)
			*status = GStatus(2006); // Error #2006: The supplied index is out of bounds.
		return 0;
	}

	return children_[index];
}

void Sprite::checkInside(float x,float y,bool visible, bool nosubs,std::vector<std::pair<int,Sprite *>> &children, std::stack<Matrix4> &pxform) const {
    float minx, miny, maxx, maxy;
    int parentidx=children.size();
    for (size_t i = 0; i < children_.size(); ++i) {
        Sprite *c=children_[i];
        Matrix transform=pxform.top() * c->localTransform_.matrix();
        c->boundsHelper(transform, &minx, &miny, &maxx, &maxy, pxform, visible, nosubs);
        if (x >= minx && y >= miny && x <= maxx && y <= maxy) {
            children.push_back(std::pair<int,Sprite *>(parentidx,c));
            if (!nosubs) {
                pxform.push(transform);
                c->checkInside(x,y,visible,nosubs,children,pxform);
                pxform.pop();
            }
        }
    }
}

void Sprite::getChildrenAtPoint(float x, float y, bool visible, bool nosubs,std::vector<std::pair<int,Sprite *>> &children) const {
	Matrix transform;
	std::stack<Matrix4> pxform;
	std::stack<const Sprite *> pstack;
    const Sprite *curr = this;
    const Sprite *last = NULL;
	while (curr) {
		pstack.push(curr);
		last = curr;
		curr = curr->parent_;
	}
	if (visible&&(!last->isStage())) return;
	while (!pstack.empty()) {
		curr=pstack.top();
		pstack.pop();
		pxform.push(transform);
		transform = transform * curr->localTransform_.matrix();
	}

    pxform.push(transform);
    checkInside(x,y,visible,nosubs,children,pxform);
}

void Sprite::removeChildAt(int index, GStatus* status) {
	if (index < 0 || index >= childCount()) {
		if (status)
			*status = GStatus(2006); // Error #2006: The supplied index is out of bounds.
		return;
	}

	void *pool = application_->createAutounrefPool();

	Sprite* child = children_[index];

	Stage *stage = child->getStage();

	if (stage)
		stage->setSpritesWithListenersDirty();

	bool connected = stage != NULL;

	child->parent_ = 0;
	children_.erase(children_.begin() + index);
	if (child->spriteWithLayoutCount) {
		Sprite *p=this;
		while (p) {
			p->spriteWithLayoutCount-=child->spriteWithLayoutCount;
			p=p->parent();
		}
	}

	application_->autounref(child);

	if (connected) {
		Event event(Event::REMOVED_FROM_STAGE);
		child->recursiveDispatchEvent(&event, false, false);
	}

	application_->deleteAutounrefPool(pool);
}

void Sprite::removeChild(Sprite* child, GStatus* status) {
	int index = getChildIndex(child, status);
	if (index == childCount()) {
		if (status)
			*status = GStatus(2025); // Error #2025: The supplied Sprite must be a child of the caller.
		return;
	}

	removeChildAt(index);
}

void Sprite::removeChild(int index, GStatus* status) {
	if (index < 0 || index >= childCount()) {
		if (status)
			*status = GStatus(2025); // Error #2025: The supplied Sprite must be a child of the caller.
		return;
	}

	removeChildAt(index);
}

bool Sprite::contains(Sprite* sprite) const {
	std::stack<const Sprite*> stack;
	stack.push(this);

	while (stack.empty() == false) {
		const Sprite* s = stack.top();
		stack.pop();

		if (s == sprite)
			return true;

		for (int i = 0; i < s->childCount(); ++i)
			stack.push(s->child(i));
	}

	return false;
}

void Sprite::replaceChild(Sprite* oldChild, Sprite* newChild) {
	// TODO: burada addedToStage ile removedFromStage'i halletmek lazim
	SpriteVector::iterator iter = std::find(children_.begin(), children_.end(),
			oldChild);

	assert(iter != children_.end());

	if (oldChild == newChild)
		return;

	oldChild->parent_ = 0;

	newChild->ref();
	oldChild->unref();
	*iter = newChild;

	newChild->parent_ = this;
}

void Sprite::localToGlobal(float x, float y, float z, float* tx, float* ty, float* tz) const {
	const Sprite* curr = this;

	while (curr) {
		curr->matrix().transformPoint(x, y, z, &x, &y, &z);
		curr = curr->parent_;
	}

	if (tx)
		*tx = x;

	if (ty)
		*ty = y;

	if (tz)
		*tz = z;
}

void Sprite::globalToLocal(float x, float y, float z, float* tx, float* ty, float* tz) const {
	std::stack<const Sprite*> stack;

	const Sprite* curr = this;
	while (curr) {
		stack.push(curr);
		curr = curr->parent_;
	}

	while (stack.empty() == false) {
		stack.top()->matrix().inverseTransformPoint(x, y, z, &x, &y, &z);
		stack.pop();
	}

	if (tx)
		*tx = x;

	if (ty)
		*ty = y;

	if (tz)
		*tz = z;
}

void Sprite::objectBounds(float* minx, float* miny, float* maxx, float* maxy,
		bool visible) const {
	std::stack<Matrix4> pxform;
	boundsHelper(Matrix(), minx, miny, maxx, maxy, pxform, visible, false);
}

#if 0
void Sprite::objectBounds(float* minx, float* miny, float* maxx, float* maxy) const
{
	Point2f min( 1e30f, 1e30f);
	Point2f max(-1e30f, -1e30f);

//	if (graphics_ != 0)
//		graphics_->bounds(&min, &max);

	float eminx, eminy, emaxx, emaxy;
	extraBounds(&eminx, &eminy, &emaxx, &emaxy);
	min.x = std::min(min.x, eminx);
	min.y = std::min(min.y, eminy);
	max.x = std::max(max.x, emaxx);
	max.y = std::max(max.y, emaxy);

	for (std::size_t i = 0; i < children_.size(); ++i)
	{
		float cminx, cminy, cmaxx, cmaxy;
		children_[i]->localBounds(&cminx, &cminy, &cmaxx, &cmaxy);

		min.x = std::min(min.x, cminx);
		min.y = std::min(min.y, cminy);
		max.x = std::max(max.x, cmaxx);
		max.y = std::max(max.y, cmaxy);
	}

	if (minx)
	*minx = min.x;
	if (miny)
	*miny = min.y;
	if (maxx)
	*maxx = max.x;
	if (maxy)
	*maxy = max.y;
}
#endif

void Sprite::localBounds(float* minx, float* miny, float* maxx, float* maxy,
		bool visible) const {
	std::stack<Matrix4> pxform;
	boundsHelper(localTransform_.matrix(), minx, miny, maxx, maxy, pxform,
			visible, false);
}

#if 0
void Sprite::localBounds(float* minx, float* miny, float* maxx, float* maxy) const
{
	float ominx, ominy, omaxx, omaxy;
	objectBounds(&ominx, &ominy, &omaxx, &omaxy);

	float lminx, lminy, lmaxx, lmaxy;

	if (ominx > omaxx || ominy > omaxy)
	{
		// empty bounds, dont transform
		lminx = ominx;
		lminy = ominy;
		lmaxx = omaxx;
		lmaxy = omaxy;
	}
	else
	{
		// transform 4 corners
		float x[4], y[4];
		matrix().transformPoint(ominx, ominy, &x[0], &y[0]);
		matrix().transformPoint(omaxx, ominy, &x[1], &y[1]);
		matrix().transformPoint(omaxx, omaxy, &x[2], &y[2]);
		matrix().transformPoint(ominx, omaxy, &x[3], &y[3]);

		// calculate local bounding box
		lminx = lmaxx = x[0];
		lminy = lmaxy = y[0];
		for (int i = 1; i < 4; ++i)
		{
			lminx = std::min(lminx, x[i]);
			lminy = std::min(lminy, y[i]);
			lmaxx = std::max(lmaxx, x[i]);
			lmaxy = std::max(lmaxy, y[i]);
		}
	}

	if (minx)
	*minx = lminx;
	if (miny)
	*miny = lminy;
	if (maxx)
	*maxx = lmaxx;
	if (maxy)
	*maxy = lmaxy;
}
#endif

#if 0
void Sprite::globalBounds(float* minx, float* miny, float* maxx, float* maxy) const
{
	float ominx, ominy, omaxx, omaxy;
	objectBounds(&ominx, &ominy, &omaxx, &omaxy);

	float gminx, gminy, gmaxx, gmaxy;

	if (ominx > omaxx || ominy > omaxy)
	{
		// empty bounds, dont transform
		gminx = ominx;
		gminy = ominy;
		gmaxx = omaxx;
		gmaxy = omaxy;
	}
	else
	{
		// transform 4 corners
		float x[4], y[4];
		localToGlobal(ominx, ominy, &x[0], &y[0]);
		localToGlobal(omaxx, ominy, &x[1], &y[1]);
		localToGlobal(omaxx, omaxy, &x[2], &y[2]);
		localToGlobal(ominx, omaxy, &x[3], &y[3]);

		// calculate global bounding box
		gminx = gmaxx = x[0];
		gminy = gmaxy = y[0];
		for (int i = 1; i < 4; ++i)
		{
			gminx = std::min(gminx, x[i]);
			gminy = std::min(gminy, y[i]);
			gmaxx = std::max(gmaxx, x[i]);
			gmaxy = std::max(gmaxy, y[i]);
		}
	}

	if (minx)
	*minx = gminx;
	if (miny)
	*miny = gminy;
	if (maxx)
	*maxx = gmaxx;
	if (maxy)
	*maxy = gmaxy;
}
#endif

float Sprite::width() const {
	float minx, maxx;
	localBounds(&minx, 0, &maxx, 0);

	if (minx > maxx)
		return 0;

	return maxx - minx;
}

float Sprite::height() const {
	float miny, maxy;
	localBounds(0, &miny, 0, &maxy);

	if (miny > maxy)
		return 0;

	return maxy - miny;
}

void Sprite::getDimensions(float& w,float &h)
{
	float minx,miny,maxx,maxy;
    extraBounds(&minx, &miny, &maxx, &maxy);
    w=(maxx>=minx)?maxx-minx:0;
    h=(maxy>=miny)?maxy-miny:0;
    if (w<reqWidth_) w=reqWidth_;
    if (h<reqHeight_) h=reqHeight_;
}

bool Sprite::hitTestPoint(float x, float y, bool visible) const {
	Matrix transform;
	std::stack<Matrix4> pxform;
	std::stack<const Sprite *> pstack;
	const Sprite *curr = this;
	const Sprite *last=NULL;
	while (curr) {
        if (visible&&(!curr->isVisible_)) return false;
        pstack.push(curr);
		last=curr;
		curr = curr->parent_;
	}
	if (visible&&(!last->isStage())) return false;

	while (!pstack.empty()) {
		curr=pstack.top();
		pstack.pop();
		pxform.push(transform);
		transform = transform * curr->localTransform_.matrix();
	}

	float tx, ty;
	//globalToLocal(x, y, &tx, &ty);
	tx = x;
	ty = y;

	float minx, miny, maxx, maxy;
	//objectBounds(&minx, &miny, &maxx, &maxy,visible);
	boundsHelper(transform, &minx, &miny, &maxx, &maxy, pxform, visible, false);

	return (tx >= minx && ty >= miny && tx <= maxx && ty <= maxy);
}

Stage *Sprite::getStage() const {
	const Sprite* curr = this;

	while (curr != NULL) {
		if (curr->isStage() == true)
			return static_cast<Stage*>(const_cast<Sprite*>(curr));

		curr = curr->parent();
	}

	return NULL;
}

void Sprite::recursiveDispatchEvent(Event* event, bool canBeStopped,
		bool reverse) {
	void *pool = application_->createAutounrefPool();

	std::vector<Sprite*> sprites;// NOTE: bunu static yapma. recursiveDispatchEvent icindeyken recursiveDispatchEvent cagirilabiliyor

	std::stack<Sprite*> stack;

	stack.push(this);

	while (stack.empty() == false) {
		Sprite* sprite = stack.top();
		stack.pop();

		sprites.push_back(sprite);

		for (int i = sprite->childCount() - 1; i >= 0; --i)
			stack.push(sprite->child(i));
	}

	if (reverse == true)
		std::reverse(sprites.begin(), sprites.end());

	for (std::size_t i = 0; i < sprites.size(); ++i) {
		sprites[i]->ref();
		application_->autounref(sprites[i]);
	}

	for (std::size_t i = 0; i < sprites.size(); ++i) {
		if (canBeStopped == false || event->propagationStopped() == false)
			sprites[i]->dispatchEvent(event);
		else
			break;
	}

	application_->deleteAutounrefPool(pool);
}

float Sprite::alpha() const {
	return alpha_;
}

struct _cliprect {
	float xmin;
	float xmax;
	float ymin;
	float ymax;
};

void Sprite::boundsHelper(const Matrix4& transform, float* minx, float* miny,
		float* maxx, float* maxy, std::stack<Matrix> parentXform,
		bool visible, bool nosubs) const {
	if ((!visible) || isVisible_) {
		this->worldTransform_ = transform;
		if (!nosubs) {
			std::stack<Sprite*> stack; // this shouldn't be static because MovieClip calls draw again
			for (size_t i = 0; i < children_.size(); ++i)
				stack.push(children_[i]);

			while (!stack.empty()) {
				Sprite *sprite = stack.top();
				stack.pop();

				sprite->worldTransform_ = sprite->parent_->worldTransform_
						* sprite->localTransform_.matrix();

				for (size_t i = 0; i < sprite->children_.size(); ++i)
					stack.push(sprite->children_[i]);
			}
		}
	}

	{
		float gminx = 1e30, gminy = 1e30, gmaxx = -1e30, gmaxy = -1e30;

		std::stack<const Sprite*> stack; // this shouldn't be static because MovieClip calls draw again
		std::stack<_cliprect> cstack;
		_cliprect noclip={-1e30,1e30,-1e30,1e30};
		stack.push(this);
		if (visible)
			cstack.push(noclip);

		while (!stack.empty()) {
			const Sprite *sprite = stack.top();
			_cliprect clip;
			stack.pop();
			if (visible)
			{
				clip= cstack.top();
				cstack.pop();
				if (!sprite->isVisible_)
					continue;
			}


			if (visible && (sprite->clipw_ >= 0) && (sprite->cliph_ >= 0)) {
				float x[4], y[4];
				//Transform clip coordinates
				sprite->worldTransform_.transformPoint(sprite->clipx_, sprite->clipy_, &x[0],
						&y[0]);
				sprite->worldTransform_.transformPoint(sprite->clipx_ + sprite->clipw_ - 1,
						sprite->clipy_, &x[1], &y[1]);
				sprite->worldTransform_.transformPoint(sprite->clipx_ + sprite->clipw_ - 1,
						sprite->clipy_ + sprite->cliph_ - 1, &x[2], &y[2]);
				sprite->worldTransform_.transformPoint(sprite->clipx_,
						sprite->clipy_ + sprite->cliph_ - 1, &x[3], &y[3]);

				float cminx = 1e30, cminy = 1e30, cmaxx = -1e30, cmaxy =
						-1e30;
				for (int i = 0; i < 4; ++i) {
					cminx = std::min(cminx, x[i]);
					cminy = std::min(cminy, y[i]);
					cmaxx = std::max(cmaxx, x[i]);
					cmaxy = std::max(cmaxy, y[i]);
				}

				clip.xmin = std::max(clip.xmin, cminx);
				clip.ymin = std::max(clip.ymin, cminy);
				clip.xmax = std::min(clip.xmax,	cmaxx);
				clip.ymax = std::min(clip.ymax,	cmaxy);
			}

			float eminx, eminy, emaxx, emaxy;
			sprite->extraBounds(&eminx, &eminy, &emaxx, &emaxy);

			if (eminx <= emaxx && eminy <= emaxy) {
				float x[4], y[4];

				float lgminx = 1e30, lgminy = 1e30, lgmaxx = -1e30, lgmaxy = -1e30;

				sprite->worldTransform_.transformPoint(eminx, eminy, &x[0],
						&y[0]);
				sprite->worldTransform_.transformPoint(emaxx, eminy, &x[1],
						&y[1]);
				sprite->worldTransform_.transformPoint(emaxx, emaxy, &x[2],
						&y[2]);
				sprite->worldTransform_.transformPoint(eminx, emaxy, &x[3],
						&y[3]);

				for (int i = 0; i < 4; ++i) {
					lgminx = std::min(lgminx, x[i]);
					lgminy = std::min(lgminy, y[i]);
					lgmaxx = std::max(lgmaxx, x[i]);
					lgmaxy = std::max(lgmaxy, y[i]);
				}


				//Clip
				if (visible)
				{
					lgminx = std::max(lgminx, clip.xmin);
					lgminy = std::max(lgminy, clip.ymin);
					lgmaxx = std::min(lgmaxx, clip.xmax);
					lgmaxy = std::min(lgmaxy, clip.ymax);
				}

				//Contribute to global bounds
				gminx = std::min(lgminx, gminx);
				gminy = std::min(lgminy, gminy);
				gmaxx = std::max(lgmaxx, gmaxx);
				gmaxy = std::max(lgmaxy, gmaxy);
			}

			if (!nosubs)
			for (size_t i = 0; i < sprite->children_.size(); ++i)
			{
				stack.push(sprite->children_[i]);
				if (visible)
					cstack.push(clip);
			}
		}

		if (visible) //Check parent hierarchy for clipping/invisibility
		{
			const Sprite *sprite = this;
			Matrix4 t = transform;
			while (sprite->parent_) {
				if (parentXform.empty())
					t = t * sprite->localTransform_.matrix().inverse();
				else {
					t = parentXform.top();
					parentXform.pop();
				}
				sprite = sprite->parent_;
				if (visible && (sprite->clipw_ >= 0) && (sprite->cliph_ >= 0)) {
					float x[4], y[4];

					t.transformPoint(sprite->clipx_, sprite->clipy_, &x[0],
							&y[0]);
					t.transformPoint(sprite->clipx_ + sprite->clipw_ - 1,
							sprite->clipy_, &x[1], &y[1]);
					t.transformPoint(sprite->clipx_ + sprite->clipw_ - 1,
							sprite->clipy_ + sprite->cliph_ - 1, &x[2], &y[2]);
					t.transformPoint(sprite->clipx_,
							sprite->clipy_ + sprite->cliph_ - 1, &x[3], &y[3]);

					float cminx = 1e30, cminy = 1e30, cmaxx = -1e30, cmaxy =
							-1e30;
					for (int i = 0; i < 4; ++i) {
						cminx = std::min(cminx, x[i]);
						cminy = std::min(cminy, y[i]);
						cmaxx = std::max(cmaxx, x[i]);
						cmaxy = std::max(cmaxy, y[i]);
					}
					gminx = std::max(gminx, cminx);
					gminy = std::max(gminy, cminy);
					gmaxx = std::min(gmaxx, cmaxx);
					gmaxy = std::min(gmaxy, cmaxy);
				}
			}
		}

		if (minx)
			*minx = gminx;
		if (miny)
			*miny = gminy;
		if (maxx)
			*maxx = gmaxx;
		if (maxy)
			*maxy = gmaxy;
	}
}

void Sprite::getBounds(const Sprite* targetCoordinateSpace, float* minx,
		float* miny, float* maxx, float* maxy) const {
	bool found = false;
	Matrix transform;
	const Sprite *curr = this;
	while (curr) {
		if (curr == targetCoordinateSpace) {
			found = true;
			break;
		}
		transform = curr->localTransform_.matrix() * transform;
		curr = curr->parent_;
	}

	if (found == false) {
		Matrix inverse;
		const Sprite *curr = targetCoordinateSpace;
		while (curr) {
			inverse = inverse * curr->localTransform_.matrix().inverse();
			curr = curr->parent_;
		}

		transform = inverse * transform;
	}

	std::stack<Matrix4> pxform;
	boundsHelper(transform, minx, miny, maxx, maxy, pxform, false, false);
}

#if 0
void Sprite::getBounds(const Sprite* targetCoordinateSpace, float* minx, float* miny, float* maxx, float* maxy)
{
	if (targetCoordinateSpace == this)
	{
		// optimization
		objectBounds(minx, miny, maxx, maxy);
		return;
	}

	float ominx, ominy, omaxx, omaxy;
	objectBounds(&ominx, &ominy, &omaxx, &omaxy);

	float gminx, gminy, gmaxx, gmaxy;

	if (ominx > omaxx || ominy > omaxy)
	{
		// empty bounds, dont transform
		gminx = ominx;
		gminy = ominy;
		gmaxx = omaxx;
		gmaxy = omaxy;
	}
	else
	{
		// transform 4 corners
		float x[4], y[4];
		this->localToGlobal(ominx, ominy, &gminx, &gminy);
		this->localToGlobal(omaxx, ominy, &gmaxx, &gminy);
		this->localToGlobal(omaxx, omaxy, &gmaxx, &gmaxy);
		this->localToGlobal(ominx, omaxy, &gminx, &gmaxy);

		targetCoordinateSpace->globalToLocal(gminx, gminy, &x[0], &y[0]);
		targetCoordinateSpace->globalToLocal(gmaxx, gminy, &x[1], &y[1]);
		targetCoordinateSpace->globalToLocal(gmaxx, gmaxy, &x[2], &y[2]);
		targetCoordinateSpace->globalToLocal(gminx, gmaxy, &x[3], &y[3]);

		// calculate bounding box
		gminx = gmaxx = x[0];
		gminy = gmaxy = y[0];
		for (int i = 1; i < 4; ++i)
		{
			gminx = std::min(gminx, x[i]);
			gminy = std::min(gminy, y[i]);
			gmaxx = std::max(gmaxx, x[i]);
			gmaxy = std::max(gmaxy, y[i]);
		}
	}

	if (minx)
	*minx = gminx;
	if (miny)
	*miny = gminy;
	if (maxx)
	*maxx = gmaxx;
	if (maxy)
	*maxy = gmaxy;
}
#endif

void Sprite::setBlendFunc(ShaderEngine::BlendFactor sfactor,
		ShaderEngine::BlendFactor dfactor) {
	sfactor_ = sfactor;
	dfactor_ = dfactor;
}

void Sprite::clearBlendFunc() {
	sfactor_ = (ShaderEngine::BlendFactor) -1;
	dfactor_ = (ShaderEngine::BlendFactor) -1;
}

void Sprite::setColorTransform(const ColorTransform& colorTransform) {
	if (colorTransform_ == 0)
		colorTransform_ = new ColorTransform();

	*colorTransform_ = colorTransform;
}

void Sprite::setAlpha(float alpha) {
	alpha_ = alpha;
}

void Sprite::eventListenersChanged() {
	Stage *stage = getStage();
	if (stage)
		stage->setSpritesWithListenersDirty();

	if (hasEventListener(Event::ENTER_FRAME))
		allSpritesWithListeners_.insert(this);
	else
		allSpritesWithListeners_.erase(this);
}

void Sprite::setStopPropagationMask(int mask) {
	stopPropagationMask_=mask;
	eventListenersChanged();
}


bool Sprite::setDimensions(float w,float h, bool forLayout)
{
//    bool changed=((reqWidth_!=w)||(reqHeight_!=h));
    G_UNUSED(forLayout);
    bool changed=(fabs(reqWidth_-w)+fabs(reqHeight_-h))>0.01;
    if (changed) {
        reqWidth_=w;
        reqHeight_=h;
        if (layoutState)
            layoutState->dirty=true;
        layoutSizesChanged();

        if (hasEventListener(LayoutEvent::RESIZED))
        {
            LayoutEvent event(LayoutEvent::RESIZED,w,h);
            dispatchEvent(&event);
        }
    }
    return changed;
}

void Sprite::set(const char* param, float value, GStatus* status) {
	int id = StringId::instance().id(param);
	set(id, value, status);
}

float Sprite::get(const char* param, GStatus* status) {
	int id = StringId::instance().id(param);
	return get(id, status);
}

void Sprite::set(int param, float value, GStatus* status) {
	switch (param) {
	case eStringIdX:
		setX(value);
		break;
	case eStringIdY:
		setY(value);
		break;
	case eStringIdZ:
		setZ(value);
		break;
	case eStringIdRotation:
		setRotation(value);
		break;
	case eStringIdRotationX:
		setRotationX(value);
		break;
	case eStringIdRotationY:
		setRotationY(value);
		break;
	case eStringIdScale:
		setScale(value);
		break;
	case eStringIdScaleX:
		setScaleX(value);
		break;
	case eStringIdScaleY:
		setScaleY(value);
		break;
	case eStringIdScaleZ:
		setScaleZ(value);
		break;
	case eStringIdAnchorX:
		setRefX(value);
		break;
	case eStringIdAnchorY:
		setRefY(value);
		break;
	case eStringIdAnchorZ:
		setRefZ(value);
		break;
	case eStringIdAlpha:
		setAlpha(value);
		break;
	case eStringIdRedMultiplier:
		setRedMultiplier(value);
		break;
	case eStringIdGreenMultiplier:
		setGreenMultiplier(value);
		break;
	case eStringIdBlueMultiplier:
		setBlueMultiplier(value);
		break;
	case eStringIdAlphaMultiplier:
		setAlphaMultiplier(value);
		break;
    case eStringIdSkewX:
        setSkewX(value);
        break;
    case eStringIdSkewY:
        setSkewY(value);
        break;
	default:
		if (status)
			*status = GStatus(2008, "param"); // Error #2008: Parameter '%s' must be one of the accepted values.
		break;
	}
}

float Sprite::get(int param, GStatus* status) {
	switch (param) {
	case eStringIdX:
		return x();
	case eStringIdY:
		return y();
	case eStringIdZ:
		return z();
	case eStringIdRotation:
		return rotation();
	case eStringIdRotationX:
		return rotationX();
	case eStringIdRotationY:
		return rotationY();
	case eStringIdScaleX:
		return scaleX();
	case eStringIdScaleY:
		return scaleY();
	case eStringIdScaleZ:
		return scaleZ();
	case eStringIdAnchorX:
		return refX();
	case eStringIdAnchorY:
		return refY();
	case eStringIdAnchorZ:
		return refZ();
	case eStringIdAlpha:
		return alpha();
	case eStringIdRedMultiplier:
		return getRedMultiplier();
	case eStringIdGreenMultiplier:
		return getGreenMultiplier();
	case eStringIdBlueMultiplier:
		return getBlueMultiplier();
	case eStringIdAlphaMultiplier:
		return getAlphaMultiplier();
    case eStringIdSkewX:
        return skewX();
    case eStringIdSkewY:
        return skewY();
	}

	if (status)
		*status = GStatus(2008, "param"); // Error #2008: Parameter '%s' must be one of the accepted values.

	return 0;
}

void Sprite::setRedMultiplier(float redMultiplier) {
	if (colorTransform_ == NULL)
		colorTransform_ = new ColorTransform();

	colorTransform_->setRedMultiplier(redMultiplier);
}

void Sprite::setGreenMultiplier(float greenMultiplier) {
	if (colorTransform_ == NULL)
		colorTransform_ = new ColorTransform();

	colorTransform_->setGreenMultiplier(greenMultiplier);
}

void Sprite::setBlueMultiplier(float blueMultiplier) {
	if (colorTransform_ == NULL)
		colorTransform_ = new ColorTransform();

	colorTransform_->setBlueMultiplier(blueMultiplier);
}

void Sprite::setAlphaMultiplier(float alphaMultiplier) {
	if (colorTransform_ == NULL)
		colorTransform_ = new ColorTransform();

	colorTransform_->setAlphaMultiplier(alphaMultiplier);
}

float Sprite::getRedMultiplier() const {
	if (colorTransform_ == NULL)
		colorTransform_ = new ColorTransform();

	return colorTransform_->redMultiplier();
}

float Sprite::getGreenMultiplier() const {
	if (colorTransform_ == NULL)
		colorTransform_ = new ColorTransform();

	return colorTransform_->greenMultiplier();
}

float Sprite::getBlueMultiplier() const {
	if (colorTransform_ == NULL)
		colorTransform_ = new ColorTransform();

	return colorTransform_->blueMultiplier();
}

float Sprite::getAlphaMultiplier() const {
	if (colorTransform_ == NULL)
		colorTransform_ = new ColorTransform();

	return colorTransform_->alphaMultiplier();
}


SpriteProxy::SpriteProxy(Application* application,void *c,SpriteProxyDraw df,SpriteProxyDestroy kf) : Sprite(application)
{
	context=c;
	drawF=df;
	destroyF=kf;
}

SpriteProxy::~SpriteProxy()
{
	destroyF(context);
}

void SpriteProxy::doDraw(const CurrentTransform& m, float sx, float sy, float ex, float ey)
{
	drawF(context,m,sx,sy,ex,ey);
}

SpriteProxy *SpriteProxyFactory::createProxy(Application* application,void *c,SpriteProxyDraw df,SpriteProxyDestroy kf) {
	return new SpriteProxy(application,c,df,kf);
}

GEventDispatcherProxy *SpriteProxyFactory::createEventDispatcher(GReferenced *c)
{
	GEventDispatcherProxy *p=new GEventDispatcherProxy();
	p->object()->setProxy(c);
	return p;
}

