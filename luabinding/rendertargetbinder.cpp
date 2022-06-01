#include "rendertargetbinder.h"
#include <grendertarget.h>
#include "luaapplication.h"
#include <luautil.h>
#include <string.h>
#include <transform.h>

RenderTargetBinder::RenderTargetBinder(lua_State *L)
{
    Binder binder(L);

    static const luaL_Reg functionList[] = {
        {"clear", clear},
        {"draw", draw},
		{"getPixel", getPixel},
		{"getPixels", getPixels},
		{"save", save},
        {NULL, NULL},
    };

    binder.createClass("RenderTarget", "TextureBase", create, destruct, functionList);
}


int RenderTargetBinder::create(lua_State *L)
{
    LuaApplication *application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);

    int width = luaL_checkinteger(L, 1);
    int height = luaL_checkinteger(L, 2);
    bool smoothing = lua_toboolean(L, 3);
    bool repeat = lua_toboolean(L, 4);
    bool selectScale = lua_toboolean(L, 5);
    bool depth = lua_toboolean(L, 6);

    Format format = eRGBA8888;
    const char *formatstr = luaL_optstring(L, 7, "rgba8888");
    if (strcmp(formatstr, "rgba8888") == 0)
        format = eRGBA8888;
    else if (strcmp(formatstr, "rgb888") == 0)
        format = eRGB888;
    else if (strcmp(formatstr, "rgb565") == 0)
        format = eRGB565;
    else if (strcmp(formatstr, "rgba4444") == 0)
        format = eRGBA4444;
    else if (strcmp(formatstr, "rgba5551") == 0)
        format = eRGBA5551;
    else if (strcmp(formatstr, "y8") == 0)
        format = eY8;
    else if (strcmp(formatstr, "a8") == 0)
        format = eA8;
    else if (strcmp(formatstr, "ya8") == 0)
        format = eYA8;
    else
    {
        GStatus status(2008, "format");		// Error #2008: Parameter %s must be one of the accepted values.
        luaL_error(L, "%s", status.errorString());
    }
    bool extend= true;
    if (!lua_isnoneornil(L,8))
        extend = lua_toboolean(L, 8);

    //Ensure requested size is never negative
    if (width <= 0) width = 0;
    if (height <= 0) height = 0;

    TextureParameters parameters;
    parameters.filter = smoothing ? eLinear : eNearest;
    parameters.wrap = repeat ? eRepeat : eClamp;
    parameters.format = format;
    parameters.pow2 = extend;

    binder.pushInstance("RenderTarget", new GRenderTarget(application->getApplication(), width, height, parameters ,selectScale,depth));

    return 1;
}

int RenderTargetBinder::destruct(void *p)
{
    void* ptr = GIDEROS_DTOR_UDATA(p);
    GReferenced* refptr = static_cast<GReferenced*>(ptr);
    refptr->unref();

    return 0;
}

int RenderTargetBinder::clear(lua_State *L)
{
    Binder binder(L);

    GRenderTarget *renderTarget = static_cast<GRenderTarget*>(binder.getInstance("RenderTarget", 1));
    unsigned int color = luaL_checkinteger(L, 2);
    float alpha = luaL_optnumber(L, 3, 1.0);
    int x=luaL_optinteger(L,4,0);
    int y=luaL_optinteger(L,5,0);
    int w=luaL_optinteger(L,6,-1);
    int h=luaL_optinteger(L,7,-1);

    renderTarget->clear(color, alpha, x, y, w, h);

    return 0;
}

int RenderTargetBinder::draw(lua_State *L)
{
    Binder binder(L);

    GRenderTarget *renderTarget = static_cast<GRenderTarget*>(binder.getInstance("RenderTarget", 1));
    Sprite *sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 2));

    Matrix4 xform;
    if (binder.isInstanceOf("Matrix",3))
    {
        Transform *mat = static_cast<Transform*>(binder.getInstance("Matrix", 3));
       	xform=mat->matrix();
    }
    else
    {
        lua_Number x=luaL_optnumber(L,3,0);
        lua_Number y=luaL_optnumber(L,4,0);
        lua_Number z=luaL_optnumber(L,5,0);
    	xform.translate(x,y,z);
    }

    renderTarget->draw(sprite,xform);

    return 0;
}

int RenderTargetBinder::getPixels(lua_State *L)
{
    Binder binder(L);

    GRenderTarget *renderTarget = static_cast<GRenderTarget*>(binder.getInstance("RenderTarget", 1));
    int x = luaL_optinteger(L, 2, 0);
    int y = luaL_optinteger(L, 3, 0);
    unsigned int w = luaL_optinteger(L, 4, renderTarget->data->width);
    unsigned int h = luaL_optinteger(L, 5, renderTarget->data->height);
    size_t bsize=w*h*renderTarget->data->parameters.bpp;

    void *buffer=malloc(bsize);
    memset(buffer,0xFF,bsize);
    if (x<0)
    {
    	w=w+x;
    	x=0;
    }
    if (y<0)
    {
    	h=h+y;
    	y=0;
    }
    if ((w>0)&&(h>0))
    	renderTarget->getPixels(x,y,w,h,buffer);
    lua_pushlstring(L,(char *)buffer,bsize);
    lua_pushinteger(L,renderTarget->data->parameters.bpp);
    free(buffer);

    return 2;
}

int RenderTargetBinder::getPixel(lua_State *L)
{
    Binder binder(L);

    GRenderTarget *renderTarget = static_cast<GRenderTarget*>(binder.getInstance("RenderTarget", 1));
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);

    unsigned char pixel[4]={0xFF,0xFF,0xFF,0xFF}; //Only works for RGBA textures

    if ((x>=0)&&(y>=0))
    	renderTarget->getPixels(x,y,1,1,pixel);
    lua_pushinteger(L,(pixel[0]<<16)|(pixel[1]<<8)|(pixel[2]<<0));
    lua_pushnumber(L,((float)pixel[3])/255.0);

    return 2;
}

int RenderTargetBinder::save(lua_State *L)
{
    Binder binder(L);

    GRenderTarget *renderTarget = static_cast<GRenderTarget*>(binder.getInstance("RenderTarget", 1));
    const char *filename=luaL_checkstring(L,2);
    int x = luaL_optinteger(L, 3, 0);
    int y = luaL_optinteger(L, 4, 0);
    unsigned int w = luaL_optinteger(L, 5, renderTarget->data->width);
    unsigned int h = luaL_optinteger(L, 6, renderTarget->data->height);
    if (x<0)
    {
    	w=w+x;
    	x=0;
    }
    if (y<0)
    {
    	h=h+y;
    	y=0;
    }
    if ((w>0)&&(h>0))
    {
    	renderTarget->save(filename,x,y,w,h);
    }
    //TODO error reporting

    return 0;
}
