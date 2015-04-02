#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"

#include "gmicrophone.h"
#include "gsoundencoder.h"

/*
  local microphone = Microphone.new(nil, 44100, 1, 16)
  microphone:setOutputFile("|D|hebe.wav")
  microphone:start()
  -- after some time
  microphone:stop()
*/

namespace {

static const char* DATA_AVAILABLE = "dataAvailable";

// some Lua helper functions
#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif

static void luaL_newweaktable(lua_State *L, const char *mode)
{
    lua_newtable(L);			// create table for instance list
    lua_pushstring(L, mode);
    lua_setfield(L, -2, "__mode");	  // set as weak-value table
    lua_pushvalue(L, -1);             // duplicate table
    lua_setmetatable(L, -2);          // set itself as metatable
}

static void luaL_rawgetptr(lua_State *L, int idx, void *ptr)
{
    idx = abs_index(L, idx);
    lua_pushlightuserdata(L, ptr);
    lua_rawget(L, idx);
}

static void luaL_rawsetptr(lua_State *L, int idx, void *ptr)
{
    idx = abs_index(L, idx);
    lua_pushlightuserdata(L, ptr);
    lua_insert(L, -2);
    lua_rawset(L, idx);
}

static char keyStrong = ' ';
static char keyWeak = ' ';

class GMicrophone : public GEventDispatcherProxy
{
public:
    GMicrophone(lua_State *L, const char *deviceName, int numChannels, int sampleRate, int bitsPerSample, gmicrophone_Error *error) : L(L)
    {
        if (++instanceCount_ == 1)
            gmicrophone_Init();

        outputFile_ = 0;

        microphone_ = gmicrophone_Create(deviceName, numChannels, sampleRate, bitsPerSample, error);

        if (microphone_ == 0)
            return;

        gmicrophone_AddCallback(microphone_, callback_s, this);

        numChannels_ = numChannels;
        sampleRate_ = sampleRate;
        bitsPerSample_ = bitsPerSample;

        started_ = false;
        paused_ = false;
    }

    ~GMicrophone()
    {
        if (microphone_)
            gmicrophone_Delete(microphone_);

        if (outputFile_)
            gsoundencoder_WavClose(outputFile_);

        if (--instanceCount_ == 0)
            gmicrophone_Cleanup();
    }

    void start(std::string* error)
    {
        if (started_)
            return;

        if (!fileName_.empty())
        {
            outputFile_ = gsoundencoder_WavCreate(fileName_.c_str(), numChannels_, sampleRate_, bitsPerSample_);

            if (outputFile_ == 0)
            {
                if (error)
                    *error = fileName_ + ": Cannot create output file.";
                return;
            }
        }

        gmicrophone_Start(microphone_);

        started_ = true;
    }

    void stop()
    {
        if (!started_)
            return;

        gmicrophone_Stop(microphone_);

        if (outputFile_)
        {
            gsoundencoder_WavClose(outputFile_);
            outputFile_ = 0;
        }

        started_ = false;
    }

    void setPaused(bool paused)
    {
        if (!started_)
            return;

        if (paused_ == paused)
            return;

        if (paused)
            gmicrophone_Stop(microphone_);
        else
            gmicrophone_Start(microphone_);

        paused_ = paused;
    }

    bool isPaused() const
    {
        return paused_;
    }

    bool isRecording() const
    {
        return started_ && !paused_;
    }

    bool isStarted() const
    {
        return started_;
    }

    void setOutputFile(const char *fileName)
    {
        if (started_)
            return;

        fileName_ = fileName;
    }

    void clearOutputFile()
    {
        if (started_)
            return;

        fileName_.clear();
    }

private:
    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<GMicrophone*>(udata)->callback(type, event);
    }

    void callback(int type, void *event)
    {
        if (type == GMICROPHONE_DATA_AVAILABLE_EVENT)
        {
            gmicrophone_DataAvailableEvent* event2 = (gmicrophone_DataAvailableEvent*)event;
            if (outputFile_)
                gsoundencoder_WavWrite(outputFile_, event2->sampleCount, event2->data);


            luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
            luaL_rawgetptr(L, -1, this);

            if (lua_isnil(L, -1))
            {
                lua_pop(L, 2);
                return;
            }

            lua_getfield(L, -1, "dispatchEvent");

            lua_pushvalue(L, -2);

            lua_getfield(L, -1, "__dataAvailableEvent");

            lua_pushnumber(L, event2->averageAmplitude);
            lua_setfield(L, -2, "averageAmplitude");

            lua_pushnumber(L, event2->peakAmplitude);
            lua_setfield(L, -2, "peakAmplitude");

            lua_call(L, 2, 0);

            lua_pop(L, 2);
        }
    }

private:
    lua_State *L;
    g_id microphone_;
    int numChannels_, sampleRate_, bitsPerSample_;
    g_id outputFile_;
    bool started_;
    bool paused_;
    std::string fileName_;
    static int instanceCount_;
};

int GMicrophone::instanceCount_ = 0;

static int create(lua_State *L)
{
    int sampleRate = luaL_checkinteger(L, 2);
    int numChannels = luaL_checkinteger(L, 3);
    int bitsPerSample = luaL_checkinteger(L, 4);

    gmicrophone_Error error;
    GMicrophone *microphone = new GMicrophone(L, NULL, numChannels, sampleRate, bitsPerSample, &error);

    switch (error)
    {
    case GMICROPHONE_NO_ERROR:
        break;
    case GMICROPHONE_CANNOT_OPEN_DEVICE:
        delete microphone;
        lua_pushnil(L);
        return 1;
    case GMICROPHONE_UNSUPPORTED_FORMAT:
        delete microphone;
        return luaL_error(L, "Unsupported microphone format.");
    }

    g_pushInstance(L, "Microphone", microphone->object());


    lua_getglobal(L, "Event");
    lua_getfield(L, -1, "new");
    lua_remove(L, -2);

    lua_pushstring(L, DATA_AVAILABLE);
    lua_call(L, 1, 1);

    lua_setfield(L, -2, "__dataAvailableEvent");


    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, microphone);
    lua_pop(L, 1);

    return 1;
}

static int destruct(lua_State *L)
{
    void* ptr = *(void**)lua_touserdata(L, 1);
    GReferenced* object = static_cast<GReferenced*>(ptr);
    GReferenced* proxy = object->proxy();
    proxy->unref();

    return 0;
}

static GMicrophone *getInstance(lua_State *L, int index)
{
    GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "Microphone", index));
    GReferenced *proxy = object->proxy();

    return static_cast<GMicrophone*>(proxy);
}

static int start(lua_State *L)
{
    GMicrophone *microphone = getInstance(L, 1);

    std::string error;
    microphone->start(&error);
    if (!error.empty())
        return luaL_error(L, error.c_str());

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyStrong);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, microphone);
    lua_pop(L, 1);

    return 0;
}

static int stop(lua_State *L)
{
    GMicrophone *microphone = getInstance(L, 1);
    microphone->stop();

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyStrong);
    lua_pushnil(L);
    luaL_rawsetptr(L, -2, microphone);
    lua_pop(L, 1);

    return 0;
}

static int lua_toboolean2(lua_State *L, int idx)
{
    if (lua_isnone(L, idx))
        return luaL_typerror(L, idx, "boolean");

    return lua_toboolean(L, idx);
}

static int setPaused(lua_State *L)
{
    GMicrophone *microphone = getInstance(L, 1);
    bool paused = lua_toboolean2(L, 2);
    microphone->setPaused(paused);

    return 0;
}

static int setOutputFile(lua_State *L)
{
    GMicrophone *microphone = getInstance(L, 1);
    const char *fileName = luaL_checkstring(L, 2);

    if (microphone->isStarted())
        return luaL_error(L, "Cannot set output file while recording.");

    microphone->setOutputFile(fileName);

    return 0;
}

static int isPaused(lua_State *L)
{
    GMicrophone *microphone = getInstance(L, 1);
    lua_pushboolean(L, microphone->isPaused());
    return 1;
}

static int isRecording(lua_State *L)
{
    GMicrophone *microphone = getInstance(L, 1);
    lua_pushboolean(L, microphone->isRecording());
    return 1;
}

static int loader(lua_State* L)
{
    const luaL_Reg functionlist[] = {
        {"start", start},
        {"stop", stop},
        {"setPaused", setPaused},
        {"isPaused", isPaused},
        {"isRecording", isRecording},
        {"setOutputFile", setOutputFile},
        {NULL, NULL},
    };

    g_createClass(L, "Microphone", "EventDispatcher", create, destruct, functionlist);

    lua_getglobal(L, "Event");
    lua_pushstring(L, DATA_AVAILABLE);
    lua_setfield(L, -2, "DATA_AVAILABLE");
    lua_pop(L, 1);

    lua_newtable(L);
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyStrong);

    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);

    lua_getglobal(L, "Microphone");
    return 1;
}

}

static void g_initializePlugin(lua_State *L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, loader);
    lua_setfield(L, -2, "microphone");

    lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
}

REGISTER_PLUGIN("Microphone", "1.0")
