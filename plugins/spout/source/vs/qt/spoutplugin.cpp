// steamplugin.cpp : Defines the exported functions for the DLL application.
//

#include "gideros.h"
#include "texturebase.h"
#include "screen.h"
#include "lua.h"
//#include "luautil.h"
#include "lauxlib.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#define GLCALL_INIT QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
#define GLCALL glFuncs->
#ifndef G_UNUSED
#define G_UNUSED(x) (void)(x)
#endif
#define G_DLLIMPORT extern "C" __declspec(dllimport)


struct SenderDetail {
	unsigned int width;
	unsigned int height;
	char name[256];
};

G_DLLIMPORT void *GidCreateSender(const char *name,int width,int height);
G_DLLIMPORT void GidDestroySender(void *s);
G_DLLIMPORT bool GidSendTexture(void *si,GLuint tid,int width,int height,GLuint fbo);
G_DLLIMPORT void *GidCreateReceiver();
G_DLLIMPORT bool GidBindReceiver(void *si,char *name,unsigned int *width,unsigned int *height, bool active);
G_DLLIMPORT void GidDestroyReceiver(void *si);
G_DLLIMPORT bool GidReceiveTexture(void *si,char *sname,GLuint tid,unsigned int *width,unsigned int *height,GLuint fbo);
G_DLLIMPORT int GidFindSenders(void *si,int bcount,struct SenderDetail *buffer);

static int sender_create(lua_State *L) {
	const char *name=luaL_checkstring(L,1);
	int width=luaL_checkinteger(L,2);
	int height=luaL_checkinteger(L,3);
	void *s=GidCreateSender(name,width,height);
	g_pushInstance(L,"SpoutSender",s);
	return 1;
}

static int sender_destruct(void *p) {
    void* ptr = GIDEROS_DTOR_UDATA(p);
    if (ptr)
    	GidDestroySender(ptr);
	return 0;
}
static int sendTexture(lua_State *L) {
	GLCALL_INIT
	void *s=g_getInstance(L,"SpoutSender",1);
	TextureBase* texture = static_cast<TextureBase*>(g_getInstance(L,"TextureBase",2));
	GLuint *tid=(GLuint *)gtexture_getInternalTexture(texture->data->gid)->getNative();
	GLint oldFBO=0;
	GLCALL glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
	lua_pushboolean(L,GidSendTexture(s,*tid,texture->data->width, texture->data->height, oldFBO));
	return 1;
}

static int receiver_create(lua_State *L) {
	void *s=GidCreateReceiver();
	g_pushInstance(L,"SpoutReceiver",s);
	return 1;
}

static int bindRec(lua_State *L) {
	void *s=g_getInstance(L,"SpoutReceiver",1);
	const char *mname=luaL_checkstring(L,2);
	bool active=lua_toboolean(L,3);
	unsigned int tw,th;
	char name[1024];
	strcpy(name,mname);
	lua_pushboolean(L,GidBindReceiver(s,name,&tw,&th,active));
	lua_pushstring(L,name);
	lua_pushnumber(L,tw);
	lua_pushnumber(L,th);
	gtexture_get_screenmanager()->screenDestroyed();
	return 4;
}

static int receiver_destruct(void *p) {
    void* ptr = GIDEROS_DTOR_UDATA(p);
    if (ptr)
    	GidDestroyReceiver(ptr);
	return 0;
}

static int receiveTexture(lua_State *L) {
	GLCALL_INIT
	void *s=g_getInstance(L,"SpoutReceiver",1);
	TextureBase* texture = static_cast<TextureBase*>(g_getInstance(L,"TextureBase",2));
	const char *mname=luaL_checkstring(L,3);
	GLuint *tid=(GLuint *)gtexture_getInternalTexture(texture->data->gid)->getNative();
	GLint oldFBO=0;
	GLCALL glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
	unsigned int tw=texture->data->width;
	unsigned int th=texture->data->height;
	char name[1024];
	strcpy(name,mname);
	lua_pushboolean(L,GidReceiveTexture(s,name,*tid,&tw,&th, oldFBO));
	lua_pushstring(L,name);
	lua_pushnumber(L,tw);
	lua_pushnumber(L,th);
	return 4;
}

static int findSenders(lua_State *L) {
	void *s=g_getInstance(L,"SpoutReceiver",1);
	int scount=GidFindSenders(s,0,NULL);
	SenderDetail *buffer=new SenderDetail[scount];
	scount=GidFindSenders(s,scount,buffer);
	lua_newtable(L);
	for (int k=0;k<scount;k++)
	{
		lua_newtable(L);
		lua_pushnumber(L,buffer[k].width); lua_setfield(L,-2,"width");
		lua_pushnumber(L,buffer[k].height); lua_setfield(L,-2,"height");
		lua_pushstring(L,buffer[k].name); lua_setfield(L,-2,"name");
		lua_rawseti(L,-2,k+1);
	}
	delete buffer;
	return 1;
}

static int loader(lua_State* L)
{
	const luaL_Reg sender_functionlist[] = {
		{"sendTexture", sendTexture},
		{NULL, NULL},
	};
	const luaL_Reg receiver_functionlist[] = {
			{"receiveTexture", receiveTexture},
			{"bind", bindRec},
			{"findSenders", findSenders},
		{NULL, NULL},
	};

	g_createClass(L, "SpoutSender", NULL, sender_create, sender_destruct, sender_functionlist);
	g_createClass(L, "SpoutReceiver", NULL, receiver_create, receiver_destruct, receiver_functionlist);

	return 0;
}

static void g_initializePlugin(lua_State* L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcnfunction(L, loader,"plugin_init_spout");
	lua_setfield(L, -2, "spout");

	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
	G_UNUSED(L);
}


REGISTER_PLUGIN("Spout", "1.0")
