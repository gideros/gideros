#include <spine/spine.h>
#include <spine/extension.h>
#include "gstdio.h"
#include "Shaders.h"
#include "application.h"
#include "lua.hpp"
#include <ogl.h>
#include <color.h>
#include <sprite.h>
#include <texturebase.h>
#include "luaapplication.h"
#include "luautil.h"
#include "gplugin.h"
#include "binder.h"
#include "gglobal.h"

#define _UNUSED(n)

#ifndef SPINE_MESH_VERTEX_COUNT_MAX
#define SPINE_MESH_VERTEX_COUNT_MAX 1000
#endif

typedef unsigned char Uint8;
static Application* application;
static lua_State *L;

Wrap wrap(spAtlasWrap wrap) {
	return wrap == SP_ATLAS_CLAMPTOEDGE ? eClamp : eRepeat;
}

Filter filter(spAtlasFilter filter) {
	switch (filter) {
	case SP_ATLAS_UNKNOWN_FILTER:
		break;
	case SP_ATLAS_NEAREST:
		return eNearest;
	case SP_ATLAS_LINEAR:
	default:
		return eLinear;
	}
	return eLinear;
}

void _spAtlasPage_createTexture(spAtlasPage* self, const char* path) {
	TextureParameters parameters;

	parameters.filter = filter(self->minFilter);
	parameters.wrap = wrap(self->uWrap);
	parameters.format = eRGBA8888;
	parameters.pow2 = false;

	TextureData *tex = application->getTextureManager()->createTextureFromFile(
			path, parameters).get();
	self->rendererObject = tex;
	self->width = tex->baseWidth;
	self->height = tex->baseHeight;
}

void _spAtlasPage_disposeTexture(spAtlasPage* self) {
	application->getTextureManager()->destroyTexture(
			(TextureData *) self->rendererObject);
}

char* _spUtil_readFile(const char* path, int* length) {
	G_FILE *f = g_fopen(path, "rb");
	if (!f)
		return 0;
	g_fseek(f, 0, SEEK_END);
	long l = g_ftell(f);
	g_fseek(f, 0, SEEK_SET);
	if (length)
		*length = l;

	char *data = (char *) malloc(l);
	g_fread(data, 1, l, f);
	g_fclose(f);

	return data;
}

_SP_ARRAY_DECLARE_TYPE(spColorArray, spColor)
_SP_ARRAY_IMPLEMENT_TYPE(spColorArray, spColor)
struct Color4u {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
	Color4u(unsigned char r1, unsigned char g1, unsigned char b1,
			unsigned char a1) :
			r(r1), g(g1), b(b1), a(a1) {
	}
	;

	Color4u(spColor vertexColor) {
		r = static_cast<Uint8>(vertexColor.r * 255);
		g = static_cast<Uint8>(vertexColor.g * 255);
		b = static_cast<Uint8>(vertexColor.b * 255);
		a = static_cast<Uint8>(vertexColor.a * 255);
	}
};

class SpineSprite {
public:
	SpineSprite(LuaApplication* application, const char *fileName,
			const char *atlasName, float scale, lua_State *L);
	virtual ~SpineSprite();
	SpriteProxy *proxy_;
	void doDraw(const CurrentTransform&, float sx, float sy, float ex,
			float ey);
	spAnimationState* state;
	spSkeleton* skeleton;
	float timeScale;
	g_id gid;
private:
	void drawMesh();
	LuaApplication* application_;
	spAtlas* atlas;
	spSkeletonData* skeletonData;
	spVertexEffect* vertexEffect;

	bool ownsAnimationStateData;
	float* worldVertices;
	spFloatArray* tempUvs;
	spColorArray* tempColors;
	spSkeletonClipping* clipper;
	bool usePremultipliedAlpha;

	double lastTime_;
	VertexBuffer<Point2f> gvertices;
	VertexBuffer<Point2f> gtexcoords;
	VertexBuffer<Color4u> gcolors;
	TextureData *ctexture;
	enum BlendMode {
		NORMAL, ADDITIVE, MULTIPLY, SCREEN
	} cblend;
};

static void b2PSS_Draw(void *c, const CurrentTransform&t, float sx, float sy,
		float ex, float ey) {
	((SpineSprite *) c)->doDraw(t, sx, sy, ex, ey);
}

static void b2PSS_Destroy(void *c) {
	delete ((SpineSprite *) c);
}

static char keyWeak = ' ';

typedef struct gspine_Event {
	int trackNumber;
	const char *trackName;
} gspine_Event;


static void callback_s(int type, void *data, void *udata) {
	const char *etype = NULL;
	switch (type) {
	case SP_ANIMATION_START:
		etype = "animationStart";
		break;
	case SP_ANIMATION_INTERRUPT:
		etype = "animationInterrupt";
		break;
	case SP_ANIMATION_END:
		etype = "animationEnd";
		break;
	case SP_ANIMATION_DISPOSE:
		etype = "animationDispose";
		break;
	case SP_ANIMATION_COMPLETE:
		etype = "animationComplete";
		break;
	default:
		return;
	}
	gspine_Event *event = (gspine_Event *) data;

    lua_checkstack(L,16);
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	luaL_rawgetptr(L, -1, udata);

	if (lua_isnil(L, -1)) {
		lua_pop(L, 2);
		return;
	}

	lua_getfield(L, -1, "dispatchEvent");
	lua_pushvalue(L, -2);
	lua_getfield(L, -1, "__spineEvent");

	lua_pushstring(L, etype);
	lua_setfield(L, -2, "__type");

	lua_pushstring(L, event->trackName);
	lua_setfield(L, -2, "animation");
	lua_pushinteger(L, event->trackNumber);
	lua_setfield(L, -2, "track");

	lua_call(L, 2, 0);

	lua_pop(L, 2);
}

void animationCallback(spAnimationState* state, spEventType type,
		spTrackEntry* entry, spEvent* event) {
	const char *animName = entry->animation ? entry->animation->name : "";
	SpineSprite *ps = ((SpineSprite*) state->rendererObject);
	switch (type) {
	case SP_ANIMATION_START:
	case SP_ANIMATION_INTERRUPT:
	case SP_ANIMATION_END:
	case SP_ANIMATION_DISPOSE:
	case SP_ANIMATION_COMPLETE:
	{
		gspine_Event *event = (gspine_Event*) gevent_CreateEventStruct1(
				sizeof(gspine_Event), offsetof(gspine_Event, trackName),
				animName);
		event->trackNumber = entry->trackIndex;

		gevent_EnqueueEvent(ps->gid, callback_s, type, event, 1, ps);
		break;
	}
	default:
		/*	case SP_ANIMATION_EVENT:
		 if (_eventListener) _eventListener(entry, event);
		 break;*/ //XXX What is this one for ?
		;
	}
}

void trackEntryCallback(spAnimationState* state, spEventType type,
		spTrackEntry* entry, spEvent* event) {
	/*	((SpineSprite*)state->rendererObject)->onTrackEntryEvent(entry, type, event);
	 if (type == SP_ANIMATION_DISPOSE)
	 if (entry->rendererObject) delete (spine::_TrackEntryListeners*)entry->rendererObject;*/
	//XXX Do nothing ? We handle event at animation state level
}

SpineSprite::SpineSprite(LuaApplication* application, const char *fileName,
		const char *atlasName, float scale, lua_State *L) :
		vertexEffect(0), worldVertices(0), clipper(0) {
	application_ = application;
	lastTime_ = 0;
	gid = g_NextId();

	atlas = spAtlas_createFromFile(atlasName, 0);
	if (!atlas) {
		lua_pushfstring(L,"Atlas file '%s' not found",atlasName);
		lua_error(L);
	}
	if (strstr(fileName, ".json") != NULL) {
		spSkeletonJson* json = spSkeletonJson_create(atlas);
		skeletonData = spSkeletonJson_readSkeletonDataFile(json, fileName);
		spSkeletonJson_dispose(json);
	} else {
		spSkeletonBinary* bin = spSkeletonBinary_create(atlas);
		skeletonData = spSkeletonBinary_readSkeletonDataFile(bin, fileName);
		spSkeletonBinary_dispose(bin);
	}
	if (!skeletonData) {
		lua_pushfstring(L,"Skeleton file '%s' not found",fileName);
		lua_error(L);
	}
	timeScale = scale;
	usePremultipliedAlpha = true;

	spBone_setYDown(true);
	worldVertices = (float *) malloc(
			SPINE_MESH_VERTEX_COUNT_MAX * sizeof(float));
	skeleton = spSkeleton_create(skeletonData);
	tempUvs = spFloatArray_create(16);
	tempColors = spColorArray_create(16);

	ownsAnimationStateData = true;

	state = spAnimationState_create(spAnimationStateData_create(skeletonData));
	state->rendererObject = this;

	clipper = spSkeletonClipping_create();
	ctexture = NULL;
	cblend = NORMAL;

	proxy_ = gtexture_get_spritefactory()->createProxy(
			application->getApplication(), this, b2PSS_Draw, b2PSS_Destroy);
	state->listener = animationCallback;
}

SpineSprite::~SpineSprite() {
	free(worldVertices);
	if (ownsAnimationStateData)
		spAnimationStateData_dispose(state->data);
	spAnimationState_dispose(state);
	spSkeleton_dispose(skeleton);
	spSkeletonClipping_dispose(clipper);
	spFloatArray_dispose(tempUvs);
	spColorArray_dispose(tempColors);
	if (skeletonData)
		spSkeletonData_dispose(skeletonData);
	spAtlas_dispose(atlas);
	delete proxy_;
	gevent_RemoveEventsWithGid(gid);
}

void SpineSprite::drawMesh() {

	ShaderProgram *p = proxy_->getShader(ShaderEngine::STDP_TEXTURECOLOR);
	if (p) {
		p->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT, 2,
				&gvertices[0], gvertices.size(), true, NULL);
		p->setData(ShaderProgram::DataColor, ShaderProgram::DUBYTE, 4,
				&gcolors[0], gcolors.size(), true, NULL);
		p->setData(ShaderProgram::DataTexture, ShaderProgram::DFLOAT, 2,
				&gtexcoords[0], gtexcoords.size(), true, NULL);

		float textureInfo[4] = { 0, 0, 0, 0 };
		if (ctexture) {
			gtexture_get_engine()->bindTexture(0,
					gtexture_getInternalTexture(ctexture->gid));
			textureInfo[0] = (float) ctexture->width
					/ (float) ctexture->exwidth;
			textureInfo[1] = (float) ctexture->height
					/ (float) ctexture->exheight;
			textureInfo[2] = 1.0 / ctexture->exwidth;
			textureInfo[3] = 1.0 / ctexture->exheight;
		}
		int sc = p->getSystemConstant(ShaderProgram::SysConst_TextureInfo);
		if (sc >= 0)
			p->setConstant(sc, ShaderProgram::CFLOAT4, 1, textureInfo);
		p->drawArrays(ShaderProgram::Triangles, 0, gvertices.size());
	}
}

void SpineSprite::doDraw(const CurrentTransform&, float _UNUSED(sx),
		float _UNUSED(sy), float _UNUSED(ex), float _UNUSED(ey)) {

	double tm = g_iclock();
	double d = tm - lastTime_;
	if (d > 1)
		d = 0;
	lastTime_ = tm;
	unsigned short quadIndices[6] = { 0, 1, 2, 2, 3, 0 };
	d *= timeScale;

	spSkeleton_update(skeleton, d);
	spAnimationState_update(state, d);
	spAnimationState_apply(state, skeleton);
	spSkeleton_updateWorldTransform(skeleton);

	if (vertexEffect != 0)
		vertexEffect->begin(vertexEffect, skeleton);

	TextureData* texture = NULL;
	BlendMode blend = NORMAL;
	for (int i = 0; i < skeleton->slotsCount; ++i) {
		spSlot* slot = skeleton->drawOrder[i];
		spAttachment* attachment = slot->attachment;
		if (!attachment)
			continue;

		float* vertices = worldVertices;
		int verticesCount = 0;
		float* uvs = 0;
		unsigned short* indices = 0;
		int indicesCount = 0;
		spColor* attachmentColor;

		if (attachment->type == SP_ATTACHMENT_REGION) {
			spRegionAttachment* regionAttachment =
					(spRegionAttachment*) attachment;
			spRegionAttachment_computeWorldVertices(regionAttachment,
					slot->bone, vertices, 0, 2);
			verticesCount = 4;
			uvs = regionAttachment->uvs;
			indices = quadIndices;
			indicesCount = 6;
			texture =
					(TextureData *) ((spAtlasRegion*) regionAttachment->rendererObject)->page->rendererObject;
			attachmentColor = &regionAttachment->color;

		} else if (attachment->type == SP_ATTACHMENT_MESH) {
			spMeshAttachment* mesh = (spMeshAttachment*) attachment;
			if (mesh->super.worldVerticesLength > SPINE_MESH_VERTEX_COUNT_MAX)
				continue;
			texture =
					(TextureData*) ((spAtlasRegion*) mesh->rendererObject)->page->rendererObject;
			spVertexAttachment_computeWorldVertices(SUPER(mesh), slot, 0,
					mesh->super.worldVerticesLength, worldVertices, 0, 2);
			verticesCount = mesh->super.worldVerticesLength >> 1;
			uvs = mesh->uvs;
			indices = mesh->triangles;
			indicesCount = mesh->trianglesCount;
			attachmentColor = &mesh->color;
		} else if (attachment->type == SP_ATTACHMENT_CLIPPING) {
			spClippingAttachment* clip =
					(spClippingAttachment*) slot->attachment;
			spSkeletonClipping_clipStart(clipper, slot, clip);
			continue;
		} else
			continue;

		Uint8 r = static_cast<Uint8>(skeleton->color.r * slot->color.r
				* attachmentColor->r * 255);
		Uint8 g = static_cast<Uint8>(skeleton->color.g * slot->color.g
				* attachmentColor->g * 255);
		Uint8 b = static_cast<Uint8>(skeleton->color.b * slot->color.b
				* attachmentColor->b * 255);
		Uint8 a = static_cast<Uint8>(skeleton->color.a * slot->color.a
				* attachmentColor->a * 255);

		spColor light;
		light.r = r / 255.0f;
		light.g = g / 255.0f;
		light.b = b / 255.0f;
		light.a = a / 255.0f;

		switch (slot->data->blendMode) {
		case SP_BLEND_MODE_NORMAL:
			blend = NORMAL;
			break;
		case SP_BLEND_MODE_ADDITIVE:
			blend = ADDITIVE;
			break;
		case SP_BLEND_MODE_MULTIPLY:
			blend = MULTIPLY;
			break;
		case SP_BLEND_MODE_SCREEN:
			blend = SCREEN;
			break;
		default:
			blend = NORMAL;
		}

		if (ctexture == 0)
			ctexture = texture;

		if (cblend != blend || ctexture != texture) {
			drawMesh();
			gvertices.clear();
			gtexcoords.clear();
			gcolors.clear();
			cblend = blend;
			ctexture = texture;
		}

		if (spSkeletonClipping_isClipping(clipper)) {
			spSkeletonClipping_clipTriangles(clipper, vertices,
					verticesCount << 1, indices, indicesCount, uvs, 2);
			vertices = clipper->clippedVertices->items;
			verticesCount = clipper->clippedVertices->size >> 1;
			uvs = clipper->clippedUVs->items;
			indices = clipper->clippedTriangles->items;
			indicesCount = clipper->clippedTriangles->size;
		}

		if (vertexEffect != 0) {
			spFloatArray_clear(tempUvs);
			spColorArray_clear(tempColors);
			for (int i = 0; i < verticesCount; i++) {
				spColor vertexColor = light;
				spColor dark;
				dark.r = dark.g = dark.b = dark.a = 0;
				int index = i << 1;
				float x = vertices[index];
				float y = vertices[index + 1];
				float u = uvs[index];
				float v = uvs[index + 1];
				vertexEffect->transform(vertexEffect, &x, &y, &u, &v,
						&vertexColor, &dark);
				vertices[index] = x;
				vertices[index + 1] = y;
				spFloatArray_add(tempUvs, u);
				spFloatArray_add(tempUvs, v);
				spColorArray_add(tempColors, vertexColor);
			}

			for (int i = 0; i < indicesCount; ++i) {
				int index = indices[i] << 1;
				Point2f v(vertices[index], vertices[index + 1]);
				Point2f t(tempUvs->items[index], tempUvs->items[index + 1]);
				Color4u c(tempColors->items[index >> 1]);
				gvertices.push_back(v);
				gtexcoords.push_back(t);
				gcolors.push_back(c);
			}
		} else {
			for (int i = 0; i < indicesCount; ++i) {
				int index = indices[i] << 1;
				Point2f v(vertices[index], vertices[index + 1]);
				Point2f t(uvs[index], uvs[index + 1]);
				Color4u c(r, g, b, a);
				gvertices.push_back(v);
				gtexcoords.push_back(t);
				gcolors.push_back(c);
			}
		}

		spSkeletonClipping_clipEnd(clipper, slot);
	}
	drawMesh();
	gvertices.clear();
	gtexcoords.clear();
	gcolors.clear();
	spSkeletonClipping_clipEnd2(clipper);

	if (vertexEffect != 0)
		vertexEffect->end(vertexEffect);
}

int createSpineSprite(lua_State *L) {
	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	::application = application->getApplication();
	Binder binder(L);
	const char *file = luaL_checkstring(L, 1);
	const char *atlas = luaL_checkstring(L, 2);
	float tscale = luaL_optnumber(L, 3, 1.0);

	SpineSprite *ps = new SpineSprite(application, file, atlas, tscale, L);

	binder.pushInstance("SpineSprite", ps->proxy_);

	lua_getglobal(L, "Event");
	lua_getfield(L, -1, "new");
	lua_remove(L, -2);

	lua_pushstring(L, "dummy");
	lua_call(L, 1, 1);

	lua_setfield(L, -2, "__spineEvent");

	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, ps);
	lua_pop(L, 1);

	return 1;
}

int destroySpineSprite(void *p) {
	return 0;
}

int setSpeed(lua_State *L) {
	Binder binder(L);
	SpriteProxy* psp = static_cast<SpriteProxy*>(binder.getInstance(
			"SpineSprite", 1));
	SpineSprite* ps = static_cast<SpineSprite*>(psp->getContext());
	ps->timeScale = luaL_checknumber(L, 2);
	return 0;
}

int getSpeed(lua_State *L) {
	Binder binder(L);
	SpriteProxy* psp = static_cast<SpriteProxy*>(binder.getInstance(
			"SpineSprite", 1));
	SpineSprite* ps = static_cast<SpineSprite*>(psp->getContext());
	lua_pushnumber(L, ps->timeScale);
	return 1;
}

int setSkin(lua_State *L) {
	Binder binder(L);
	SpriteProxy* psp = static_cast<SpriteProxy*>(binder.getInstance(
			"SpineSprite", 1));
	SpineSprite* ps = static_cast<SpineSprite*>(psp->getContext());
	lua_pushboolean(L,spSkeleton_setSkinByName(ps->skeleton, luaL_optstring(L, 2,NULL)));
	return 1;
}

int setAttachment(lua_State *L) {
	Binder binder(L);
	SpriteProxy* psp = static_cast<SpriteProxy*>(binder.getInstance(
			"SpineSprite", 1));
	SpineSprite* ps = static_cast<SpineSprite*>(psp->getContext());
	lua_pushboolean(L,spSkeleton_setAttachment(ps->skeleton, luaL_checkstring(L, 2),
			luaL_optstring(L, 3,NULL)));
	return 1;
}

int getBoneLocation(lua_State *L) {
	Binder binder(L);
	SpriteProxy* psp = static_cast<SpriteProxy*>(binder.getInstance(
			"SpineSprite", 1));
	SpineSprite* ps = static_cast<SpineSprite*>(psp->getContext());
	spBone *bone = spSkeleton_findBone(ps->skeleton, luaL_checkstring(L, 2));
	if (bone == NULL) {
		lua_pushfstring(L, "No bone named '%s' in skeleton",
				luaL_checkstring(L, 2));
		lua_error(L);
	}
	float lx = luaL_optnumber(L, 3, 0);
	float ly = luaL_optnumber(L, 4, 0);
	float lr = luaL_optnumber(L, 5, 0);
	float wx, wy, wr;
	spBone_localToWorld(bone, lx, ly, &wx, &wy);
	wr = spBone_localToWorldRotation(bone, lr);
	lua_pushnumber(L, wx);
	lua_pushnumber(L, wy);
	lua_pushnumber(L, wr);
	return 3;
}

int setAnimation(lua_State *L) {
	Binder binder(L);
	SpriteProxy* psp = static_cast<SpriteProxy*>(binder.getInstance(
			"SpineSprite", 1));
	SpineSprite* ps = static_cast<SpineSprite*>(psp->getContext());
	spTrackEntry *te = NULL;
	if (lua_isnoneornil(L, 3))
		te = spAnimationState_setEmptyAnimation(ps->state,
				luaL_checkinteger(L, 2), luaL_optnumber(L, 4, 0));
	else {
		te = spAnimationState_setAnimationByName(ps->state,
				luaL_checkinteger(L, 2), luaL_checkstring(L, 3),
				lua_toboolean(L, 5));
		if ((!lua_isnoneornil(L, 4)) && te)
			te->mixDuration = luaL_checknumber(L, 4);
	}
	if (te)
		te->rendererObject = ps;

	return 0;
}

int addAnimation(lua_State *L) {
	Binder binder(L);
	SpriteProxy* psp = static_cast<SpriteProxy*>(binder.getInstance(
			"SpineSprite", 1));
	SpineSprite* ps = static_cast<SpineSprite*>(psp->getContext());
	spTrackEntry *te = NULL;
	if (lua_isnoneornil(L, 3))
		te = spAnimationState_addEmptyAnimation(ps->state,
				luaL_checkinteger(L, 2), luaL_optnumber(L, 4, 0),
				luaL_optnumber(L, 6, 0.0));
	else {
		te = spAnimationState_addAnimationByName(ps->state,
				luaL_checkinteger(L, 2), luaL_checkstring(L, 3),
				lua_toboolean(L, 5), luaL_optnumber(L, 6, 0.0));
		if ((!lua_isnoneornil(L, 4)) && te)
			te->mixDuration = luaL_checknumber(L, 4);
	}
	if (te)
		te->rendererObject = ps;

	return 0;
}

int stopAnimation(lua_State *L) {
	Binder binder(L);
	SpriteProxy* psp = static_cast<SpriteProxy*>(binder.getInstance(
			"SpineSprite", 1));
	SpineSprite* ps = static_cast<SpineSprite*>(psp->getContext());
	spTrackEntry *te = spAnimationState_setEmptyAnimation(ps->state,
			luaL_checkinteger(L, 2), luaL_optnumber(L, 3, 0));
	if (te)
		te->rendererObject = ps;
	return 0;
}

int setMix(lua_State *L) {
	Binder binder(L);
	SpriteProxy* psp = static_cast<SpriteProxy*>(binder.getInstance(
			"SpineSprite", 1));
	SpineSprite* ps = static_cast<SpineSprite*>(psp->getContext());
	if (lua_isnone(L, 3))
		ps->state->data->defaultMix = luaL_checknumber(L, 2);
	else
		spAnimationStateData_setMixByName(ps->state->data,
				luaL_checkstring(L, 2), luaL_checkstring(L, 3),
				luaL_checknumber(L, 4));
	return 0;
}

int loader(lua_State *L) {
//StackChecker checker(L, "Box2DBinder2::loader", 1);

	Binder binder(L);

	const luaL_Reg SpineSprite_functionList[] = {
			{ "setAnimation", setAnimation },
			{ "stopAnimation", stopAnimation },
			{ "addAnimation", addAnimation }, { "setMix", setMix }, {
					"setSpeed", setSpeed }, { "getSpeed", getSpeed }, {
					"setAttachment", setAttachment }, //(slotName,attachment)
			{ "setSkin", setSkin }, { "getBoneLocation", getBoneLocation }, {
					NULL, NULL }, };
	binder.createClass("SpineSprite", "Sprite", createSpineSprite,
			destroySpineSprite, SpineSprite_functionList);

	lua_getglobal(L, "Event");
	lua_pushstring(L, "animationStart");
	lua_setfield(L, -2, "ANIMATION_START");
	lua_pushstring(L, "animationInterrupt");
	lua_setfield(L, -2, "ANIMATION_INTERRUPT");
	lua_pushstring(L, "animationEnd");
	lua_setfield(L, -2, "ANIMATION_END");
	lua_pushstring(L, "animationDispose");
	lua_setfield(L, -2, "ANIMATION_DISPOSE");
	lua_pushstring(L, "animationComplete");
	lua_setfield(L, -2, "ANIMATION_COMPLETE");
	lua_pop(L, 1);

    luaL_newweaktable(L);
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);

	return 1;
}

static void g_initializePlugin(lua_State *L) {
	::L = L;
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcnfunction(L, loader,"plugin_init_spine");
	lua_setfield(L, -2, "spine");

	lua_pop(L, 2);

}

static void g_deinitializePlugin(lua_State *_UNUSED(L)) {
}

#ifdef QT_NO_DEBUG
REGISTER_PLUGIN_NAMED("Spine", "1.1.0", spine)
#elif defined(TARGET_OS_MAC) || defined(_MSC_VER)
REGISTER_PLUGIN_STATICNAMED_CPP("Spine", "1.1.0", spine)
#else
REGISTER_PLUGIN_NAMED("Spine", "1.1.0", spine)
#endif
