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

#if 0
template<class T> class faststack {
private:
	std::vector<std::vector<T*>> data;
	size_t first_index, last_index;
	size_t lsize = 0;
	const size_t chunk = 1024;
public:
	faststack() {
		first_index = 0;
		last_index = 0;
		data.reserve(16);
	}
	T* pop_front()
	{
		if ((lsize <= 1) && (first_index == last_index)) return nullptr;
		T* r = data[0][first_index++];
		if (first_index == chunk) {
			data.erase(data.begin());
			first_index = 0;
			lsize--;
		}
		return r;
	}
	T* pop()
	{
		if ((lsize <= 1) && (first_index == last_index)) return nullptr;
		if (last_index == 0)
			last_index = chunk - 1;
		else
			last_index--;

		T* r = data[lsize-1][last_index];
		if (last_index == 0) {
			data.erase(data.end()-1);
			lsize--;
		}
		return r;
	}
	void push(T* val)
	{
		if (last_index == 0) {
			lsize++;
			data.resize(lsize);
			data[lsize - 1].resize(chunk);
		}
		data[lsize - 1][last_index++] = val;
		if (last_index == chunk) last_index = 0;
	}
	void push_all(T** vals, size_t count)
	{
		while (count > 0) {
			if (last_index == 0) {
				lsize++;
				data.resize(lsize);
				data[lsize - 1].resize(chunk);
			}
			size_t max = chunk - last_index;
			if (max > count) max = count;

			T** vc = data[lsize - 1].data() + last_index;
			size_t cp = max;
			while (cp--)
				*(vc++) = *(vals++);
			//memcpy(, vals, sizeof(T*) * max);
			last_index += max;
			count -= max;

			if (last_index == chunk) last_index = 0;
		}
	}
};
#else
template<class T> class faststack {
	std::stack<T *> s;
public:
	T* pop()
	{
        if (s.empty()) return nullptr;
		T *r=s.top();
        s.pop();
        return r;
	}
	void push(T* v) { s.push(v); };
	void push_all(T** vals, size_t count)
	{
		while ((count--) > 0)
			s.push(*(vals++));
	}
};
#endif

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
	checkClip_=false;
    changes_=(ChangeSet)0x0FF; //All invalid, except constraints which is never actually revalidated
	hasCustomShader_=false;
}

void Sprite::cloneFrom(Sprite *s) {
    effectsMode_=s->effectsMode_;
    hasCustomShader_=s->hasCustomShader_;
    effectStack_=s->effectStack_;
    skipSet_=s->skipSet_;
    checkClip_=s->checkClip_;
    memcpy(boundsCache,s->boundsCache,sizeof(boundsCache));
    changes_=s->changes_;
    if (s->layoutConstraints)
    {
        layoutConstraints=getLayoutConstraints();
        *layoutConstraints=*s->layoutConstraints;
    }
    if (s->layoutState)
    {
        layoutState=getLayoutState();
        *layoutState=*s->layoutState;
    }
    spriteWithLayoutCount=s->spriteWithLayoutCount;
    isVisible_=s->isVisible_;
    localTransform_=s->localTransform_;
    worldTransform_=s->worldTransform_;
    sfactor_=s->sfactor_;
    dfactor_=s->dfactor_;
    for (auto ss=s->children_.cbegin();ss!=s->children_.cend();ss++) {
        Sprite *sc=(*ss)->clone();
        sc->parent_=this;
        children_.push_back(sc);
    }
    colorTransform_=s->colorTransform_;
    alpha_=s->alpha_;
    clipx_=s->clipx_;
    clipy_=s->clipy_;
    clipw_=s->clipw_;
    cliph_=s->cliph_;
    reqWidth_=s->reqWidth_;
    reqHeight_=s->reqHeight_;
    stopPropagationMask_=s->stopPropagationMask_;
    shaders_=s->shaders_;
    for (auto ss=shaders_.cbegin();ss!=shaders_.cend();ss++)
        if (ss->second.shader) ss->second.shader->Retain();
    stencil_=s->stencil_;
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
	invalidate(INV_GRAPHICS|INV_SHADER);
}

bool Sprite::setShaderConstant(ShaderParam p,ShaderEngine::StandardProgram id,int variant)
{
	int sid=(id<<8)|variant;
	std::map<int,struct _ShaderSpec>::iterator it=shaders_.find(sid);
	if (it!=shaders_.end()) {
		it->second.params[p.name]=p;
		invalidate(INV_GRAPHICS);
		return true;
	}
	else
		return false;
}

ShaderProgram *Sprite::getShader(ShaderEngine::StandardProgram id,int variant)
{
	if (hasCustomShader_||(changes_&INV_SHADER)) {
		hasCustomShader_=false;
		revalidate(INV_SHADER);
		int sid=(id<<8)|variant;
		std::map<int, struct _ShaderSpec>::iterator it;
		if (!shaders_.empty()) {
			hasCustomShader_=true;
			it = shaders_.find(sid);
			if (it != shaders_.end()) {
				setupShader(it->second);
				return it->second.shader;
			}
			it = shaders_.find(0);
			if (it != shaders_.end()) {
				setupShader(it->second);
				return it->second.shader;
			}
		}
		Sprite *p=parent();
		while (p) {
			if (!p->shaders_.empty()) {
				hasCustomShader_=true;
				it = p->shaders_.find(sid);
				if (it != p->shaders_.end()) {
					if (it->second.inherit) {
						setupShader(it->second);
						return it->second.shader;
					}
				}
			}
			p=p->parent();
		}
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
	invalidate(INV_EFFECTS);
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
	invalidate(INV_EFFECTS);
	return true;
}

void Sprite::updateEffects()
{
	if (effectsMode_!=CONTINUOUS) {
		if (!(changes_&INV_EFFECTS)) return;
	}
	effectsDrawing_=true;
    float swidth,sheight;
	if (!effectStack_.empty()) {
		float minx, miny, maxx, maxy;

        objectBounds(&minx, &miny, &maxx, &maxy,true);

		if (minx > maxx || miny > maxy)
			return; //Empty Sprite, do nothing
		swidth = maxx;
		sheight = maxy;
		for (size_t i = 0; i < effectStack_.size(); i++) {
			if (effectStack_[i].buffer) {
				if (i == 0) { //First stage, draw the Sprite normally onto the first buffer
					Matrix xform;
					if (effectStack_[i].autoBuffer) {
						float maxx, maxy;

						effectStack_[i].autoTransform.transformPoint(0, 0, &maxx, &maxy);
						float tx, ty;
						effectStack_[i].autoTransform.transformPoint(swidth, 0, &tx, &ty);
						maxx = std::max(maxx, tx); maxy = std::max(maxy, ty);
						effectStack_[i].autoTransform.transformPoint(0, sheight, &tx, &ty);
						maxx = std::max(maxx, tx); maxy = std::max(maxy, ty);
						effectStack_[i].autoTransform.transformPoint(swidth, sheight, &tx, &ty);
						maxx = std::max(maxx, tx); maxy = std::max(maxy, ty);
						maxx = std::max(maxx, .0F); maxy = std::max(maxy, .0F);
						swidth = maxx; sheight = maxy;
                        float sx = application_->getLogicalScaleX()/application_->getScale();
                        float sy = application_->getLogicalScaleY()/application_->getScale();
                        swidth *= sx;
                        sheight *= sy;
                        int bw=ceil(swidth);
                        int bh=ceil(sheight);
                        effectStack_[i].buffer->resize(bw, bh, sx, sy);
                       // xform.scale(1.0/sx, 1.0/sy, 1);
					}
					if (effectStack_[i].clearBuffer)
						effectStack_[i].buffer->clear(0, 0, 0, 0, -1, -1);
					xform = xform * effectStack_[i].transform;
					Matrix invL = localTransform_.matrix().inverse();
					xform = xform * invL;

					effectStack_[i].buffer->draw(this, xform);
				}
				else if (effectStack_[i - 1].buffer) {
					if (effectStack_[i].autoBuffer) {
						float maxx, maxy;

						effectStack_[i].autoTransform.transformPoint(0, 0, &maxx, &maxy);
						float tx, ty;
						effectStack_[i].autoTransform.transformPoint(swidth, 0, &tx, &ty);
						maxx = std::max(maxx, tx); maxy = std::max(maxy, ty);
						effectStack_[i].autoTransform.transformPoint(0, sheight, &tx, &ty);
						maxx = std::max(maxx, tx); maxy = std::max(maxy, ty);
						effectStack_[i].autoTransform.transformPoint(swidth, sheight, &tx, &ty);
						maxx = std::max(maxx, tx); maxy = std::max(maxy, ty);
						maxx = std::max(maxx, .0F); maxy = std::max(maxy, .0F);
						swidth = maxx; sheight = maxy;
						effectStack_[i].buffer->resize(ceilf(swidth), ceilf(sheight), application_->getLogicalScaleX(), application_->getLogicalScaleY());
					}
					if (effectStack_[i].clearBuffer)
						effectStack_[i].buffer->clear(0, 0, 0, 0, -1, -1);
					Bitmap source(application_, effectStack_[i - 1].buffer);
					setupEffectShader(&source, effectStack_[i - 1]);
					effectStack_[i].buffer->draw(&source, effectStack_[i].transform);
				}
			}
		}
	}
	revalidate(INV_EFFECTS);
	effectsDrawing_=false;
}

void Sprite::redrawEffects() {
	invalidate(INV_EFFECTS);
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
    invalidate(INV_LAYOUT);
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
    invalidate(INV_CONSTRAINTS);
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
            invalidate(INV_CONSTRAINTS);
        }
    }
}


void Sprite::childrenDrawn() {
}

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
	static GGPool<faststack<Sprite> > stackPool;
	faststack<Sprite>& stack = *stackPool.create();

	stack.push(this);

	while (true) {
		Sprite* sprite = stack.pop();
		bool pop = false;
		if (sprite == nullptr) {
			pop = true;
			sprite = stack.pop();
			if (sprite == nullptr)
				break;
		}

		bool lastEffect=((!sprite->effectsDrawing_)&&(sprite->effectStack_.size()>0));

		if (pop == true) {
            sprite->drawCount_=1;
            for (size_t i = 0;i< sprite->children_.size();i++)
                sprite->drawCount_+=sprite->children_[i]->drawCount_;
            sprite->childrenDrawn();
            sprite->revalidate(INV_GRAPHICS);
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

        int clipState=checkClip_?ShaderEngine::Engine->hasClip():-1;
        if (clipState>0)
            continue;

		if ((sprite != this) && (sprite->parent_))
			sprite->worldTransform_ = sprite->parent_->worldTransform_ * sprite->localTransform_.matrix();
		else
			sprite->worldTransform_ = transform * localTransform_.matrix();

        ShaderEngine::Engine->setModel(sprite->worldTransform_);

        if (clipState==0) {
            float minx,miny,maxx,maxy;
            sprite->extraBounds(&minx, &miny, &maxx, &maxy);
            if ((maxx>=minx)&&(maxy>miny)&&ShaderEngine::Engine->checkClip(minx,miny,maxx-minx,maxy-miny))
                    continue;
        }


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
			stencil.cullMode=sprite->stencil_.cullMode;
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
            xform=xform*sprite->localTransform_.matrix();
            xform=xform*sprite->effectStack_[i].postTransform;
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

		stack.push(sprite);
		stack.push(nullptr); //End marker

        if (!lastEffect) //Don't draw subs if rendering last effect
        {
            int sc=sprite->skipSet_.size();
            char *sd=sprite->skipSet_.data();
			int sz = sprite->children_.size();
            for (int i = sz - 1; i >= 0; --i)
            {
                if ((i>=sc)||(!sd[i]))
                    stack.push(sprite->children_[i]);
            }
        }
	}

	stackPool.destroy(&stack);
}

void Sprite::logicalTransformChanged()
{
    changes_=(ChangeSet)(changes_|INV_EFFECTS);
    for (size_t i = 0; i < children_.size(); ++i)
        children_[i]->logicalTransformChanged();
}

void Sprite::computeLayout() {
	if (!spriteWithLayoutCount) return;
	static GGPool<faststack<Sprite>> stackPool;
	faststack<Sprite> &stack = *stackPool.create();

	stack.push(this);

	while (true) {
		Sprite *sprite = stack.pop();
		if (sprite == nullptr) break;

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

		stack.push_all(sprite->children_.data(),sprite->children_.size());
		if (!sprite->effectStack_.empty())
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

int Sprite::addChild(Sprite* sprite, GStatus* status) {
    return addChildAt(sprite, childCount(), status);
}

int Sprite::addChildAt(Sprite* sprite, int index, GStatus* status) {
	/* This is not necessary, we are only called by spritebinder and the check is already done there.
	if (canChildBeAddedAt(sprite, index, status) == false)
		return -1;
	*/

    invalidate(INV_GRAPHICS|INV_BOUNDS|INV_LAYOUT);
	Stage* stage1 = sprite->getStage();

	if (stage1)
		stage1->setSpritesWithListenersDirty();

	if (sprite->parent_ == this) {
        auto it=std::find(children_.begin(), children_.end(), sprite);
        size_t cindex=it-children_.begin();
        children_.erase(it);
        if (cindex<(size_t)index) index--;
        children_.insert(children_.begin() + index, sprite);
        return index;
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
    if (layoutState&&sprite->layoutConstraints) {
        layoutState->dirty=true;
        layoutState->layoutInfoCache[0].valid=false;
        layoutState->layoutInfoCache[1].valid=false;
    }

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
    return index;
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

void Sprite::checkInside(float x,float y,bool visible, bool nosubs,std::vector<std::pair<int,Sprite *>> &children, std::stack<Matrix4> &pxform, bool xformValid) const {
    float minx, miny, maxx, maxy;
    int parentidx=children.size();
    size_t sc=skipSet_.size();
    const char *sd=skipSet_.data();
    for (size_t i = 0; i < children_.size(); ++i) {
        if ((i>=sc)||(!sd[i])) {
            Sprite *c=children_[i];
            if (c->isVisible_) {
                Matrix transform=pxform.top() * c->localTransform_.matrix();
                c->boundsHelper(transform, &minx, &miny, &maxx, &maxy, pxform, visible, nosubs, BOUNDS_GLOBAL, &xformValid);
                if (x >= minx && y >= miny && x <= maxx && y <= maxy) {
                    children.push_back(std::pair<int,Sprite *>(parentidx,c));
                    if ((!nosubs)&&(!c->children_.empty())) {
                        pxform.push(transform);
                        c->checkInside(x,y,visible,nosubs,children,pxform,xformValid); //We are recursing so matrix must have been set already
                        pxform.pop();
                    }
                }
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
    invalidate(INV_GRAPHICS|INV_BOUNDS|INV_LAYOUT);

	void *pool = application_->createAutounrefPool();

	Sprite* child = children_[index];
    child->invalidate(INV_CONSTRAINTS);

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

    oldChild->invalidate(INV_CONSTRAINTS);
    oldChild->parent_ = 0;

	newChild->ref();
	oldChild->unref();
	*iter = newChild;

	newChild->parent_ = this;
	invalidate(INV_GRAPHICS|INV_BOUNDS);
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
        bool visible) {
	std::stack<Matrix4> pxform;
    boundsHelper(Matrix(), minx, miny, maxx, maxy, pxform, visible, false, BOUNDS_OBJECT);
}

void Sprite::localBounds(float* minx, float* miny, float* maxx, float* maxy,
        bool visible) {
	std::stack<Matrix4> pxform;
	boundsHelper(localTransform_.matrix(), minx, miny, maxx, maxy, pxform,
            visible, false, BOUNDS_LOCAL);
}

float Sprite::width() {
	float minx, maxx;
	localBounds(&minx, 0, &maxx, 0);

	if (minx > maxx)
		return 0;

	return maxx - minx;
}

float Sprite::height() {
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

bool Sprite::hitTestPoint(float x, float y, bool visible) {
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
    boundsHelper(transform, &minx, &miny, &maxx, &maxy, pxform, visible, false, BOUNDS_GLOBAL);

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

void Sprite::invalidate(int changes) {

	if (changes&(INV_VISIBILITY))
        changes|=INV_LAYOUT|INV_CONSTRAINTS;

	if (changes&(INV_CLIP|INV_TRANSFORM|INV_VISIBILITY))
		changes|=INV_BOUNDS;

    if (changes&(INV_BOUNDS))
        changes|=INV_GRAPHICS;

    if ((changes_&changes)==changes) return; //Already invalid

    int downchanges=changes&(INV_TRANSFORM|INV_BOUNDS|INV_SHADER); //Bound, transfrom and shader changes impact children
	if (downchanges) {
		faststack<Sprite> stack;
		stack.push_all(children_.data(),children_.size());
		while (true) {
			Sprite *h=stack.pop();
			if (h==nullptr) break;
			h->changes_=(ChangeSet)(h->changes_|downchanges);
			stack.push_all(h->children_.data(),h->children_.size());
		}
	}

	if (changes&(INV_GRAPHICS))
		changes|=INV_EFFECTS;


    changes_=(ChangeSet)(changes_|(changes&(~INV_CONSTRAINTS)));

	changes=(ChangeSet)(changes&~(INV_VISIBILITY|INV_CLIP|INV_TRANSFORM|INV_GRAPHICS|INV_SHADER));
    if (changes&(INV_CONSTRAINTS))
        changes|=INV_LAYOUT;

	//Propagate to parents
    Sprite *h=parent_;
	while (changes&&h) {
        if (h->layoutState)
        {
            if (changes&(INV_LAYOUT|INV_CONSTRAINTS))
                if (!h->layoutState->optimizing)
                    h->layoutState->dirty=true;
            if (changes&INV_CONSTRAINTS) {
                h->layoutState->layoutInfoCache[0].valid=false;
                h->layoutState->layoutInfoCache[1].valid=false;
            }
        }
		else
            changes=(ChangeSet)(changes&(~(INV_LAYOUT|INV_CONSTRAINTS)));

        h->changes_=(ChangeSet)(h->changes_|(changes&(~INV_CONSTRAINTS)));
		h=h->parent_;
	}
}

struct _cliprect {
	float xmin;
	float xmax;
	float ymin;
	float ymax;
};

void Sprite::boundsHelper(const Matrix4& transform, float* minx, float* miny,
		float* maxx, float* maxy, std::stack<Matrix> parentXform,
        bool visible, bool nosubs, BoundsMode mode, bool *xformValid) {
    if (changes_&INV_BOUNDS) {
        for (size_t i=0;i<BOUNDS_MAX*4;i++)
            boundsCache[i].valid=false;
        revalidate(INV_BOUNDS);
    }
    int cacheMode=(mode<<2)+(visible?2:0)+(nosubs?1:0);

    if (boundsCache[cacheMode].valid) {
        if (minx)
            *minx=boundsCache[cacheMode].minx;
        if (miny)
            *miny=boundsCache[cacheMode].miny;
        if (maxx)
            *maxx=boundsCache[cacheMode].maxx;
        if (maxy)
            *maxy=boundsCache[cacheMode].maxy;
        return;
    }

    //Validate transform
    if ((!visible) || isVisible_) {
		this->worldTransform_ = transform;
        if (!(nosubs||(xformValid&&(*xformValid)))) {
			faststack<Sprite> stack;
			stack.push_all((Sprite **) (children_.data()), children_.size());

			while (true) {
				Sprite *sprite = stack.pop();
				if (sprite == nullptr) break;

                if ((!visible) || sprite->isVisible_) {
                    sprite->worldTransform_ = sprite->parent_->worldTransform_
                            * sprite->localTransform_.matrix();

                    if (!visible)
                        stack.push_all(sprite->children_.data(),sprite->children_.size());
                    else {
						int sc=sprite->skipSet_.size();
						const char *sd=sprite->skipSet_.data();
						int sz = sprite->children_.size();
						for (int i = 0; i <sz; i++)
						{
							if ((i>=sc)||(!sd[i]))
								stack.push(sprite->children_[i]);
						}
                    }
                }
			}
            if (xformValid) *xformValid=true;
		}
	}

    //Compute bounds
	{
		float gminx = 1e30, gminy = 1e30, gmaxx = -1e30, gmaxy = -1e30;

		faststack<const Sprite> stack;
		std::stack<_cliprect> cstack;
		_cliprect noclip={-1e30,1e30,-1e30,1e30};
		stack.push(this);
		if (visible)
			cstack.push(noclip);

		//Gather children bounds
		while (true) {
			const Sprite *sprite = stack.pop();
			if (sprite == nullptr) break;
			_cliprect clip;
			if (visible)
			{
				clip= cstack.top();
				cstack.pop();
				if (!sprite->isVisible_)
					continue;

                if ((sprite->clipw_ >= 0) && (sprite->cliph_ >= 0)) {
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

			if (!nosubs) {
				int sc=sprite->skipSet_.size();
				const char *sd=sprite->skipSet_.data();
				int sz = sprite->children_.size();
				for (int i = 0; i <sz; i++)
				{
					if (visible) {
						if ((i>=sc)||(!sd[i]))
							stack.push(sprite->children_[i]);
						cstack.push(clip);
					}
					else {
						stack.push(sprite->children_[i]);
					}
				}
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

        boundsCache[cacheMode].minx=gminx;
        boundsCache[cacheMode].miny=gminy;
        boundsCache[cacheMode].maxx=gmaxx;
        boundsCache[cacheMode].maxy=gmaxy;

		if (minx)
			*minx = gminx;
		if (miny)
			*miny = gminy;
		if (maxx)
			*maxx = gmaxx;
		if (maxy)
			*maxy = gmaxy;
	}

    if (mode==BOUNDS_UNSPEC) return;
    boundsCache[cacheMode].valid=true;
}

void Sprite::getBounds(const Sprite* targetCoordinateSpace, float* minx,
        float* miny, float* maxx, float* maxy) {
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
    boundsHelper(transform, minx, miny, maxx, maxy, pxform, false, false,BOUNDS_UNSPEC);
}

void Sprite::setBlendFunc(ShaderEngine::BlendFactor sfactor,
		ShaderEngine::BlendFactor dfactor) {
	sfactor_ = sfactor;
	dfactor_ = dfactor;
	invalidate(INV_GRAPHICS);
}

void Sprite::clearBlendFunc() {
	sfactor_ = (ShaderEngine::BlendFactor) -1;
	dfactor_ = (ShaderEngine::BlendFactor) -1;
	invalidate(INV_GRAPHICS);
}

void Sprite::setColorTransform(const ColorTransform& colorTransform) {
	if (colorTransform_ == 0)
		colorTransform_ = new ColorTransform();

	*colorTransform_ = colorTransform;
	invalidate(INV_GRAPHICS);
}

void Sprite::setAlpha(float alpha) {
	alpha_ = alpha;
	invalidate(INV_GRAPHICS);
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
        invalidate(INV_CONSTRAINTS);
        layoutSizesChanged();
        redrawEffects();

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
	invalidate(INV_GRAPHICS);
}

void Sprite::setGreenMultiplier(float greenMultiplier) {
	if (colorTransform_ == NULL)
		colorTransform_ = new ColorTransform();

	colorTransform_->setGreenMultiplier(greenMultiplier);
	invalidate(INV_GRAPHICS);
}

void Sprite::setBlueMultiplier(float blueMultiplier) {
	if (colorTransform_ == NULL)
		colorTransform_ = new ColorTransform();

	colorTransform_->setBlueMultiplier(blueMultiplier);
	invalidate(INV_GRAPHICS);
}

void Sprite::setAlphaMultiplier(float alphaMultiplier) {
	if (colorTransform_ == NULL)
		colorTransform_ = new ColorTransform();

	colorTransform_->setAlphaMultiplier(alphaMultiplier);
	invalidate(INV_GRAPHICS);
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

