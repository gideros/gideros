#include "urlloaderbinder.h"
#include <eventdispatcher.h>
#include "luautil.h"
#include "stackchecker.h"
#include <gstatus.h>
#include <ghttp.h>
#include <string>

static inline bool ishex(char c)
{
    return	('0' <= c && c <= '9') ||
            ('a' <= c && c <= 'f') ||
            ('A' <= c && c <= 'F');
}

static std::string encode(const char* url)
{
    std::string eurl;

    size_t len = strlen(url);

    for (size_t i = 0; i < len; ++i)
    {
        bool notencoded = false;
        if (url[i] == 0x25)
        {
            if (i + 2 < len)
            {
                if (!ishex(url[i + 1]) || !ishex(url[i + 2]))
                    notencoded = true;
            }
            else
            {
                notencoded = true;
            }
        }

        // we get this from from QUrl:toEncoded
        if (url[i] == 0x20 ||	// ' '
            url[i] == 0x22 ||	// '"'
//			url[i] == 0x25 ||	// '%'
            url[i] == 0x3C ||	// '<'
            url[i] == 0x3E ||	// '>'
            url[i] == 0x5B ||	// '['
            url[i] == 0x5C ||	// '\'
            url[i] == 0x5D ||	// ']'
            url[i] == 0x5E ||	// '^'
            url[i] == 0x60 ||	// '`'
            url[i] == 0x7B ||	// '{'
            url[i] == 0x7C ||	// '|'
            url[i] == 0x7D ||	// '}'
            url[i] == 0x7F ||	// control character
            notencoded ||
            (unsigned char)url[i] >= 128)
        {
            char buf[4];
            sprintf(buf, "%c%X%X", 0x25, (url[i] & 0xf0) >> 4, url[i] & 0x0f);
            eurl += buf;
        }
        else
        {
            eurl += url[i];
        }
    }

    return eurl;
}

#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif

static const char *GET = "get";
static const char *POST = "post";
static const char *PUT = "put";
static const char *DELETE = "delete";

static char keyWeak = ' ';
static char keyStrong = ' ';

static Event::Type COMPLETE("complete");
static Event::Type ERROR("error");
static Event::Type PROGRESS("progress");

class GGUrlLoader : public EventDispatcher
{
public:
    GGUrlLoader(lua_State *L) : L(L)
    {
        id_ = 0;
    }

    ~GGUrlLoader()
    {
        close();
    }

    void get(const char *url, ghttp_Header *headers)
    {
        close();
        std::string eurl = encode(url);
        id_ = ghttp_Get(eurl.c_str(), headers, callback_s, this);
    }

    void post(const char *url, ghttp_Header *headers, const void *body, size_t size)
    {
        close();
        std::string eurl = encode(url);
        id_ = ghttp_Post(eurl.c_str(), headers, body, size, callback_s, this);
    }

    void put(const char *url, ghttp_Header *headers, const void *body, size_t size)
    {
        close();
        std::string eurl = encode(url);
        id_ = ghttp_Put(eurl.c_str(), headers, body, size, callback_s, this);
    }

    void deleteResource(const char *url, ghttp_Header *headers)
    {
        close();
        std::string eurl = encode(url);
        id_ = ghttp_Delete(eurl.c_str(), headers, callback_s, this);
    }

    void close()
    {
        if (id_ != 0)
        {
            ghttp_Close(id_);
            id_ = 0;
        }
    }

private:
    static void callback_s(int type, void *data, void *udata)
    {
        static_cast<GGUrlLoader*>(udata)->callback(type, data);
    }

    void getOrCreateEvent(const char *type, const char *field)
    {
        lua_getfield(L, -1, field);

        if (lua_isnil(L, -1))
        {
            lua_pop(L, 1);

            lua_getglobal(L, "Event");
            lua_getfield(L, -1, "new");
            lua_remove(L, -2);

            lua_pushstring(L, type);
            lua_call(L, 1, 1);

            lua_pushvalue(L, -1);
            lua_setfield(L, -3, field);
        }
    }

    void callback(int type, void *data)
    {
        if (type == GHTTP_RESPONSE_EVENT || type == GHTTP_ERROR_EVENT)
        {
            id_ = 0;
        }

        luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
        luaL_rawgetptr(L, -1, this);

        if (lua_isnil(L, -1))
        {
            lua_pop(L, 2);
            return;
        }

        if (type == GHTTP_RESPONSE_EVENT || type == GHTTP_ERROR_EVENT)
        {
            luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyStrong);
            lua_pushvalue(L, -2);
            lua_pushnil(L);
            lua_settable(L, -3);
            lua_pop(L, 1);
        }

        if ((type == GHTTP_RESPONSE_EVENT && !hasEventListener(COMPLETE)) ||
            (type == GHTTP_ERROR_EVENT && !hasEventListener(ERROR)) ||
            (type == GHTTP_PROGRESS_EVENT && !hasEventListener(PROGRESS)))
        {
            lua_pop(L, 2);
            return;
        }

        lua_getfield(L, -1, "dispatchEvent");

        lua_pushvalue(L, -2);

        if (type == GHTTP_RESPONSE_EVENT)
        {
            getOrCreateEvent("complete", "__completeEvent");

            ghttp_ResponseEvent* d = (ghttp_ResponseEvent*)data;
            lua_pushlstring(L, (char*)d->data, d->size);
            lua_setfield(L, -2, "data");
            if (d->httpStatusCode != -1)
            {
                lua_pushinteger(L, d->httpStatusCode);
                lua_setfield(L, -2, "httpStatusCode");
            }
           	lua_newtable(L);
           	int hdr=0;
           	while (d->headers[hdr].name)
           	{
           		lua_pushstring(L,d->headers[hdr].value);
           		lua_setfield(L,-2,d->headers[hdr].name);
				hdr++;
           	}
       		lua_setfield(L,-2,"headers");
        }
        else if (type == GHTTP_ERROR_EVENT)
        {
            getOrCreateEvent("error", "__errorEvent");
        }
        else if (type == GHTTP_PROGRESS_EVENT)
        {
            getOrCreateEvent("progress", "__progressEvent");

            ghttp_ProgressEvent* d = (ghttp_ProgressEvent*)data;
            lua_pushinteger(L, d->bytesLoaded);
            lua_setfield(L, -2, "bytesLoaded");
            lua_pushinteger(L, d->bytesTotal);
            lua_setfield(L, -2, "bytesTotal");
        }

        lua_call(L, 2, 0);

        lua_pop(L, 2);
    }

private:
    lua_State *L;
    int id_;
};

UrlLoaderBinder::UrlLoaderBinder(lua_State* L)
{
    Binder binder(L);

    static const luaL_Reg functionList[] = {
        {"load", load},
        {"close", close},
        {"ignoreSslErrors", ignoreSslErrors},
        {"setProxy", setProxy},
        {NULL, NULL},
    };

    binder.createClass("UrlLoader", "EventDispatcher", create, destruct, functionList);

    lua_getglobal(L, "UrlLoader");

    lua_pushstring(L, GET);
    lua_setfield(L, -2, "GET");

    lua_pushstring(L, POST);
    lua_setfield(L, -2, "POST");

    lua_pushstring(L, PUT);
    lua_setfield(L, -2, "PUT");

    lua_pushstring(L, DELETE);
    lua_setfield(L, -2, "DELETE");

    lua_pop(L, 1);
}

static void load(lua_State* L,
                 GGUrlLoader* urlloader,
                 int index)
{
    if (!lua_isnoneornil(L, index))
    {
        const char* url = luaL_checkstring(L, index);

        int method = 0;
        if (!lua_isnoneornil(L, index + 1))
        {
            const char* methodstr = luaL_checkstring(L, index + 1);

            if (strcmp(methodstr, GET) == 0)
                method = 0;
            else if (strcmp(methodstr, POST) == 0)
                method = 1;
            else if (strcmp(methodstr, PUT) == 0)
                method = 2;
            else if (strcmp(methodstr, DELETE) == 0)
                method = 3;
            else
            {
                GStatus status(2010, "method");	// Error #2010 "Field %s must be one of the accepted values."
                luaL_error(L, status.errorString());
            }
        }

        std::vector<std::pair<std::string, std::string> > header;
        std::vector<ghttp_Header> header2;
        ghttp_Header *header3 = NULL;
        if (lua_type(L, index + 2) == LUA_TTABLE)
        {
            int t = abs_index(L, index + 2);
            lua_pushnil(L);
            while (lua_next(L, t) != 0)
            {
                lua_pushvalue(L, -2);
                std::string key = luaL_checkstring(L, -1);
                lua_pop(L, 1);

                std::string value = luaL_checkstring(L, -1);

                header.push_back(std::make_pair(key, value));

                lua_pop(L, 1);
            }

            header2.resize(header.size());
            for (size_t i = 0; i < header.size(); ++i)
            {
                header2[i].name = header[i].first.c_str();
                header2[i].value = header[i].second.c_str();
            }

            ghttp_Header null = {NULL, NULL};
            header2.push_back(null);

            header3 = &header2[0];
        }

        if (method == 0)
        {
            if (urlloader)
                urlloader->get(url, header3);
        }
        else if (method == 1)
        {
            const char* data = NULL;
            size_t size = 0;
            int di = header3 ? 3 : 2;
            if (!lua_isnoneornil(L, index + di))
                data = luaL_checklstring(L, index + di, &size);

            if (urlloader)
                urlloader->post(url, header3, data, size);
        }
        else if (method == 2)
        {
            const char* data = NULL;
            size_t size = 0;
            int di = header3 ? 3 : 2;
            if (!lua_isnoneornil(L, index + di))
                data = luaL_checklstring(L, index + di, &size);

            if (urlloader)
                urlloader->put(url, header3, data, size);
        }
        else if (method == 3)
        {
            if (urlloader)
                urlloader->deleteResource(url, header3);
        }
    }
}

int UrlLoaderBinder::create(lua_State* L)
{
    Binder binder(L);

    ::load(L, NULL, 1);     // error checking before creating GGUrlLoader

    GGUrlLoader* urlloader = new GGUrlLoader(L);
    ::load(L, urlloader, 1);

    binder.pushInstance("UrlLoader", urlloader);

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyStrong);
    if (lua_isnil(L, -1))
    {
        lua_newtable(L);
        luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyStrong);
    }
    lua_pop(L, 1);

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    if (lua_isnil(L, -1))
    {
        luaL_newweaktable(L);
        luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    }
    lua_pop(L, 1);

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, urlloader);
    lua_pop(L, 1);

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyStrong);
    lua_pushvalue(L, -2);
    lua_pushboolean(L, 1);
    lua_settable(L, -3);
    lua_pop(L, 1);

    return 1;
}

int UrlLoaderBinder::destruct(lua_State* L)
{
    void* ptr = *(void**)lua_touserdata(L, 1);
    GGUrlLoader* urlloader = static_cast<GGUrlLoader*>(ptr);
    urlloader->unref();

    return 0;
}

int UrlLoaderBinder::load(lua_State* L)
{
    Binder binder(L);
    GGUrlLoader* urlloader = static_cast<GGUrlLoader*>(binder.getInstance("UrlLoader", 1));

    ::load(L, urlloader, 2);

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyStrong);
    lua_pushvalue(L, 1);
    lua_pushboolean(L, 1);
    lua_settable(L, -3);
    lua_pop(L, 1);

    return 0;
}

int UrlLoaderBinder::ignoreSslErrors(lua_State* L)
{
	ghttp_IgnoreSSLErrors();
	return 0;
}

int UrlLoaderBinder::setProxy(lua_State* L)
{
	const char *host=luaL_optstring(L,1,NULL);
	int port=luaL_optinteger(L,2,80);
	const char *user=luaL_optstring(L,3,NULL);
	const char *pass=luaL_optstring(L,4,NULL);
	ghttp_SetProxy(host, port, user, pass);
	return 0;
}

int UrlLoaderBinder::close(lua_State* L)
{
    Binder binder(L);
    GGUrlLoader* urlloader = static_cast<GGUrlLoader*>(binder.getInstance("UrlLoader", 1));

    urlloader->close();

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyStrong);
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    lua_settable(L, -3);
    lua_pop(L, 1);

    return 0;
}
