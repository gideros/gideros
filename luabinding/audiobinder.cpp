#include "audiobinder.h"
#include <eventdispatcher.h>
#include "luautil.h"
#include <gaudio.h>
#include <gpath.h>
#include <sys/stat.h>
#include "stackchecker.h"
#include <gstdio.h>
#include <completeevent.h>
#include "luaapplication.h"

static char keyStrong = ' ';
static char keySound = ' ';

namespace {

struct GGSoundInterface
{
    void (*SoundDelete)(g_id sound);
    unsigned int (*SoundGetLength)(g_id sound);

    g_id (*SoundPlay)(g_id sound, g_bool paused);
    void (*ChannelStop)(g_id channel);
    void (*ChannelSetPosition)(g_id channel, unsigned int position);
    unsigned int (*ChannelGetPosition)(g_id channel);
    void (*ChannelSetPaused)(g_id channel, g_bool paused);
    g_bool (*ChannelIsPaused)(g_id channel);
    g_bool (*ChannelIsPlaying)(g_id channel);
    void (*ChannelSetVolume)(g_id channel, float volume);
    float (*ChannelGetVolume)(g_id channel);
    void (*ChannelSetPitch)(g_id channel, float pitch);
    float (*ChannelGetPitch)(g_id channel);
    void (*ChannelSetLooping)(g_id channel, g_bool looping);
    g_bool (*ChannelIsLooping)(g_id channel);
    void (*ChannelSetWorldPosition)(g_id channel,float x,float y,float z,float vx,float vy,float vz);
    g_id (*ChannelAddCallback)(g_id channel, gevent_Callback callback, void *udata);
    void (*ChannelRemoveCallback)(g_id channel, gevent_Callback callback, void *udata);
    void (*ChannelRemoveCallbackWithGid)(g_id channel, g_id gid);
};

class GGSound : public GReferenced
{
public:
    GGSound(lua_State *L, const char *fileName, gaudio_Error *error, const std::vector<char> &sig) :
        L(L),
        sig_(sig)
    {
        /* TODO: create a better way to get main thread */
        LuaApplication *application = (LuaApplication*)luaL_getdata(L);
        lua_State *mainL = application->getLuaState();
        L = mainL;
        this->L = mainL;

        const char *dot = strrchr(fileName, '.');

        std::string dot2 = dot ? (dot + 1) : "";
        std::transform(dot2.begin(), dot2.end(), dot2.begin(), ::tolower);

        if (dot2 == "wav")
        {
            gid = gaudio_SoundCreateFromFile(fileName, g_false, error);
            setSoundInterface();
        }
        else
        {
            if ((dot2=="mp3") && gaudio_BackgroundMusicIsAvailable())
            {
                gid = gaudio_BackgroundMusicCreateFromFile(fileName, error);
                setBackgroundMusicInterface();
            }
            else
            {
                gid = gaudio_SoundCreateFromFile(fileName, g_true, error);
                setSoundInterface();
            }
        }
       /* else
        {
            gid = 0;

            G_FILE *fis = g_fopen(fileName, "rb");
            if (fis)
                g_fclose(fis);

            if (error)
                *error = fis ? GAUDIO_UNSUPPORTED_FORMAT : GAUDIO_CANNOT_OPEN_FILE;
        }*/

        if (!sig_.empty())
        {
            luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keySound);
            lua_pushlstring(L, &sig_[0], sig_.size());
            lua_pushlightuserdata(L, this);
            lua_rawset(L, -3);
            lua_pop(L, 1);
        }
    }

    ~GGSound()
    {
        if (!sig_.empty())
        {
            luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keySound);
            lua_pushlstring(L, &sig_[0], sig_.size());
            lua_pushnil(L);
            lua_rawset(L, -3);
            lua_pop(L, 1);
        }

        if (gid)
            interface.SoundDelete(gid);
    }

    unsigned int getLength()
    {
        return interface.SoundGetLength(gid);
    }

    g_id gid;
    lua_State *L;
    GGSoundInterface interface;

private:
    std::vector<char> sig_;

private:
    void setSoundInterface()
    {
        interface.SoundDelete = gaudio_SoundDelete;
        interface.SoundGetLength = gaudio_SoundGetLength;
        interface.SoundPlay = gaudio_SoundPlay;
        interface.ChannelStop = gaudio_ChannelStop;
        interface.ChannelSetPosition = gaudio_ChannelSetPosition;
        interface.ChannelGetPosition = gaudio_ChannelGetPosition;
        interface.ChannelSetPaused = gaudio_ChannelSetPaused;
        interface.ChannelIsPaused = gaudio_ChannelIsPaused;
        interface.ChannelIsPlaying = gaudio_ChannelIsPlaying;
        interface.ChannelSetVolume = gaudio_ChannelSetVolume;
        interface.ChannelGetVolume = gaudio_ChannelGetVolume;
        interface.ChannelSetPitch = gaudio_ChannelSetPitch;
        interface.ChannelGetPitch = gaudio_ChannelGetPitch;
        interface.ChannelSetLooping = gaudio_ChannelSetLooping;
        interface.ChannelIsLooping = gaudio_ChannelIsLooping;
        interface.ChannelSetWorldPosition = gaudio_ChannelSetWorldPosition;
        interface.ChannelAddCallback = gaudio_ChannelAddCallback;
        interface.ChannelRemoveCallback = gaudio_ChannelRemoveCallback;
        interface.ChannelRemoveCallbackWithGid = gaudio_ChannelRemoveCallbackWithGid;
    }

    void setBackgroundMusicInterface()
    {
        interface.SoundDelete = gaudio_BackgroundMusicDelete;
        interface.SoundGetLength = gaudio_BackgroundMusicGetLength;
        interface.SoundPlay = gaudio_BackgroundMusicPlay;
        interface.ChannelStop = gaudio_BackgroundChannelStop;
        interface.ChannelSetPosition = gaudio_BackgroundChannelSetPosition;
        interface.ChannelGetPosition = gaudio_BackgroundChannelGetPosition;
        interface.ChannelSetPaused = gaudio_BackgroundChannelSetPaused;
        interface.ChannelIsPaused = gaudio_BackgroundChannelIsPaused;
        interface.ChannelIsPlaying = gaudio_BackgroundChannelIsPlaying;
        interface.ChannelSetVolume = gaudio_BackgroundChannelSetVolume;
        interface.ChannelGetVolume = gaudio_BackgroundChannelGetVolume;
        interface.ChannelSetPitch = NULL;
        interface.ChannelGetPitch = NULL;
        interface.ChannelSetLooping = gaudio_BackgroundChannelSetLooping;
        interface.ChannelIsLooping = gaudio_BackgroundChannelIsLooping;
        interface.ChannelSetWorldPosition = NULL;
        interface.ChannelAddCallback = gaudio_BackgroundChannelAddCallback;
        interface.ChannelRemoveCallback = gaudio_BackgroundChannelRemoveCallback;
        interface.ChannelRemoveCallbackWithGid = gaudio_BackgroundChannelRemoveCallbackWithGid;
    }
};


class GGSoundChannel : public EventDispatcher
{
public:
    GGSoundChannel(lua_State *L, GGSound *sound, unsigned int startTime, bool looping, bool paused) :
        L(L), sound_(sound)
    {
        /* TODO: create a better way to get main thread */
        LuaApplication *application = (LuaApplication*)luaL_getdata(L);
        lua_State *mainL = application->getLuaState();
        L = mainL;
        this->L = mainL;

        interface = sound->interface;

        sound_->ref();

        gid = interface.SoundPlay(sound->gid, g_true);

        if (gid == 0)
            return;

        volume_ = 1;
        pitch_ = 1;
        lastPosition_ = 0;

        interface.ChannelAddCallback(gid, callback_s, this);

        if (startTime != 0)
            interface.ChannelSetPosition(gid, startTime);

        looping_ = looping;
        interface.ChannelSetLooping(gid, looping);

        paused_ = paused;
        if (paused == false)
            interface.ChannelSetPaused(gid, g_false);
    }

    ~GGSoundChannel()
    {
        if (gid)
            interface.ChannelStop(gid);

        sound_->unref();
    }

    void stop()
    {
        if (gid)
        {
            lastPosition_ = interface.ChannelGetPosition(gid);
            interface.ChannelStop(gid);
            gid = 0;
        }
    }

    unsigned int getPosition()
    {
        if (gid == 0)
            return lastPosition_;

        return interface.ChannelGetPosition(gid);
    }

    void setPosition(unsigned int position)
    {
        if (gid)
            interface.ChannelSetPosition(gid, position);
    }

    void setVolume(float volume)
    {
        volume_ = volume;

        if (gid)
            interface.ChannelSetVolume(gid, volume);
    }

    float getVolume()
    {
        return volume_;
    }

    void setPitch(float pitch)
    {
        pitch_ = pitch;

        if (gid)
            interface.ChannelSetPitch(gid, pitch);
    }

    float getPitch()
    {
        return pitch_;
    }

    bool isPitchingAvailable()
    {
        return interface.ChannelSetPitch != NULL;
    }

    bool isPlaying()
    {
        if (gid == 0)
            return false;

        return interface.ChannelIsPlaying(gid);
    }

    void setPaused(bool paused)
    {
        paused_ = paused;

        if (gid)
            interface.ChannelSetPaused(gid, paused);
    }

    bool isPaused()
    {
        return paused_;
    }

    void setLooping(bool looping)
    {
        looping_ = looping;

        if (gid)
            interface.ChannelSetLooping(gid, looping);
    }

    bool isLooping()
    {
        return looping_;
    }

    void setWorldPosition(float x,float y,float z,float vx,float vy,float vz)
    {
        if (gid&&interface.ChannelSetWorldPosition)
        	interface.ChannelSetWorldPosition(gid,x,y,z,vx,vy,vz);
    }



    lua_State *L;
    g_id gid;
    GGSoundInterface interface;

private:
    GGSound *sound_;
    float volume_;
    float pitch_;
    unsigned int lastPosition_;
    bool paused_;
    bool looping_;

private:
    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<GGSoundChannel*>(udata)->callback(type, event);
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

    void callback(int type, void *event)
    {
        if (type == GAUDIO_CHANNEL_COMPLETE_EVENT)
        {
            lastPosition_ = interface.ChannelGetPosition(gid);
            gid = 0;

            CompleteEvent event(CompleteEvent::COMPLETE);
            dispatchEvent(&event);

            luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyStrong);
            lua_pushnil(L);
            luaL_rawsetptr(L, -2, this);
            lua_pop(L, 1);
        }
    }
};

}

AudioBinder::AudioBinder(lua_State *L)
{
    Binder binder(L);

    const luaL_Reg Sound_functionList[] = {
        {"play", Sound_play},
        {"getLength", Sound_getLength},
        {"setListenerPosition", Sound_setListenerPosition},
        {NULL, NULL},
    };

    binder.createClass("Sound", NULL, Sound_create, Sound_destruct, Sound_functionList);

    const luaL_Reg SoundChannel_functionList[] = {
        {"stop", SoundChannel_stop},
        {"setPosition", SoundChannel_setPosition},
        {"getPosition", SoundChannel_getPosition},
        {"setVolume", SoundChannel_setVolume},
        {"getVolume", SoundChannel_getVolume},
        {"setPitch", SoundChannel_setPitch},
        {"getPitch", SoundChannel_getPitch},
        {"isPlaying", SoundChannel_isPlaying},
        {"setPaused", SoundChannel_setPaused},
        {"isPaused", SoundChannel_isPaused},
        {"setLooping", SoundChannel_setLooping},
        {"isLooping", SoundChannel_isLooping},
        {"setWorldPosition", SoundChannel_setWorldPosition},
        {NULL, NULL},
    };

    binder.createClass("SoundChannel", "EventDispatcher", NULL, SoundChannel_destruct, SoundChannel_functionList);

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyStrong);
    if (lua_isnil(L, -1))
    {
        lua_newtable(L);
        luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyStrong);
    }
    lua_pop(L, 1);

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keySound);
    if (lua_isnil(L, -1))
    {
        lua_newtable(L);
        luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keySound);
    }
    lua_pop(L, 1);
}

static void append(std::vector<char>& buffer, const void *data, size_t len)
{
    size_t s = buffer.size();
    buffer.resize(s + len);
    memcpy(&buffer[s], data, len);
}

int AudioBinder::Sound_create(lua_State *L)
{
    StackChecker checker(L, "AudioBinder::Sound_create", 1);

    Binder binder(L);

    const char *fileName = luaL_checkstring(L, 1);

    std::vector<char> sig;
    int flags = gpath_getDriveFlags(gpath_getPathDrive(fileName));
    if (flags & GPATH_RO)
    {
        append(sig, fileName, strlen(fileName) + 1);
    }
    else
    {
        if (flags & GPATH_REAL)
        {
            append(sig, fileName, strlen(fileName) + 1);

            struct stat s;
            stat(gpath_transform(fileName), &s);
            append(sig, &s.st_mtime, sizeof(s.st_mtime));
        }
    }

    GGSound *sound = NULL;

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keySound);

    if (sig.empty())
    {
        lua_pushnil(L);
    }
    else
    {
        lua_pushlstring(L, &sig[0], sig.size());
        lua_rawget(L, -2);
    }

    if (!lua_isnil(L, -1))
    {
        sound = static_cast<GGSound*>(lua_touserdata(L, -1));
        sound->ref();
    }
    else
    {
        gaudio_Error error;
        sound = new GGSound(L, fileName, &error, sig);

        switch (error)
        {
        case GAUDIO_NO_ERROR:
            break;
        case GAUDIO_CANNOT_OPEN_FILE:
            sound->unref();
            luaL_error(L, "%s: No such file or directory.", fileName);
            break;
        case GAUDIO_UNRECOGNIZED_FORMAT:
            sound->unref();
            luaL_error(L, "%s: Sound format is not recognized.", fileName);
            break;
        case GAUDIO_ERROR_WHILE_READING:
            sound->unref();
            luaL_error(L, "%s: Error while reading.", fileName);
            break;
        case GAUDIO_UNSUPPORTED_FORMAT:
            sound->unref();
            luaL_error(L, "%s: Sound format is not supported.", fileName);
            break;
        case GAUDIO_INTERNAL_ERROR:
            sound->unref();
            luaL_error(L, "%s: Sound internal error.", fileName);
            break;
        }
    }

    lua_pop(L, 2);

    binder.pushInstance("Sound", sound);

    return 1;
}

int AudioBinder::Sound_destruct(lua_State *L)
{
    void *ptr = *(void**)lua_touserdata(L, 1);
    GGSound *sound = static_cast<GGSound*>(ptr);
    sound->unref();

    return 0;
}

int AudioBinder::Sound_getLength(lua_State *L)
{
    Binder binder(L);
    GGSound *sound = static_cast<GGSound*>(binder.getInstance("Sound"));

    lua_pushinteger(L, sound->getLength());

    return 1;
}

int AudioBinder::Sound_setListenerPosition(lua_State *L)
{
	gaudio_SoundListener(luaL_optnumber(L, 1,0.0),luaL_optnumber(L, 2,0.0),luaL_optnumber(L, 3,0.0),
			luaL_optnumber(L, 4,0.0),luaL_optnumber(L, 5,0.0),luaL_optnumber(L, 6,0.0),
			luaL_optnumber(L, 7,0.0),luaL_optnumber(L, 8,0.0),luaL_optnumber(L, 9,0.0),
			luaL_optnumber(L, 10,0.0),luaL_optnumber(L, 11,0.0),luaL_optnumber(L, 12,0.0));
    return 0;
}

int AudioBinder::Sound_play(lua_State *L)
{
    Binder binder(L);

    GGSound *sound = static_cast<GGSound*>(binder.getInstance("Sound", 1));

    unsigned int startTime = luaL_optnumber(L, 2, 0);

    bool looping;
    if (lua_type(L, 3) == LUA_TNUMBER)
    {
        // backward compatibility
        looping = lua_tonumber(L, 3) > 1;
    }
    else
    {
        looping = lua_toboolean(L, 3);
    }

    bool paused = lua_toboolean(L, 4);

    GGSoundChannel *soundChannel = new GGSoundChannel(L, sound, startTime, looping, paused);

    if (soundChannel->gid == 0)
    {
        soundChannel->unref();
        lua_pushnil(L);
    }
    else
    {
        binder.pushInstance("SoundChannel", soundChannel);

        luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyStrong);
        lua_pushvalue(L, -2);
        luaL_rawsetptr(L, -2, soundChannel);
        lua_pop(L, 1);
    }

    return 1;
}

int AudioBinder::SoundChannel_destruct(lua_State *L)
{
    void *ptr = *(void**)lua_touserdata(L, 1);
    GGSoundChannel *soundChannel = static_cast<GGSoundChannel*>(ptr);
    soundChannel->unref();

    return 0;
}


int AudioBinder::SoundChannel_stop(lua_State *L)
{
    Binder binder(L);

    GGSoundChannel *soundChannel = static_cast<GGSoundChannel*>(binder.getInstance("SoundChannel", 1));

    soundChannel->stop();

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyStrong);
    lua_pushnil(L);
    luaL_rawsetptr(L, -2, soundChannel);
    lua_pop(L, 1);

    return 0;
}

int AudioBinder::SoundChannel_setPosition(lua_State *L)
{
    Binder binder(L);

    GGSoundChannel *soundChannel = static_cast<GGSoundChannel*>(binder.getInstance("SoundChannel", 1));

    unsigned int position = luaL_checknumber(L, 2);

    soundChannel->setPosition(position);

    return 0;
}

int AudioBinder::SoundChannel_getPosition(lua_State *L)
{
    Binder binder(L);

    GGSoundChannel *soundChannel = static_cast<GGSoundChannel*>(binder.getInstance("SoundChannel", 1));

    lua_pushinteger(L, soundChannel->getPosition());

    return 1;
}

int AudioBinder::SoundChannel_setVolume(lua_State *L)
{
    Binder binder(L);

    GGSoundChannel *soundChannel = static_cast<GGSoundChannel*>(binder.getInstance("SoundChannel", 1));

    soundChannel->setVolume(luaL_checknumber(L, 2));

    return 0;
}

int AudioBinder::SoundChannel_getVolume(lua_State *L)
{
    Binder binder(L);

    GGSoundChannel *soundChannel = static_cast<GGSoundChannel*>(binder.getInstance("SoundChannel", 1));

    lua_pushnumber(L, soundChannel->getVolume());

    return 1;
}

int AudioBinder::SoundChannel_setPitch(lua_State *L)
{
    Binder binder(L);

    GGSoundChannel *soundChannel = static_cast<GGSoundChannel*>(binder.getInstance("SoundChannel", 1));

    if (!soundChannel->isPitchingAvailable())
        return luaL_error(L, "Sound pitching is not supported for background music.");

    soundChannel->setPitch(luaL_checknumber(L, 2));

    return 0;
}

int AudioBinder::SoundChannel_getPitch(lua_State *L)
{
    Binder binder(L);

    GGSoundChannel *soundChannel = static_cast<GGSoundChannel*>(binder.getInstance("SoundChannel", 1));

    if (!soundChannel->isPitchingAvailable())
        return luaL_error(L, "Sound pitching is not supported for background music.");

    lua_pushnumber(L, soundChannel->getPitch());

    return 1;
}

int AudioBinder::SoundChannel_isPlaying(lua_State *L)
{
    Binder binder(L);

    GGSoundChannel *soundChannel = static_cast<GGSoundChannel*>(binder.getInstance("SoundChannel", 1));

    lua_pushboolean(L, soundChannel->isPlaying());

    return 1;
}

int AudioBinder::SoundChannel_setPaused(lua_State *L)
{
    Binder binder(L);

    GGSoundChannel *soundChannel = static_cast<GGSoundChannel*>(binder.getInstance("SoundChannel", 1));

    soundChannel->setPaused(lua_toboolean2(L, 2));

    return 0;
}

int AudioBinder::SoundChannel_isPaused(lua_State *L)
{
    Binder binder(L);

    GGSoundChannel *soundChannel = static_cast<GGSoundChannel*>(binder.getInstance("SoundChannel", 1));

    lua_pushboolean(L, soundChannel->isPaused());

    return 1;
}

int AudioBinder::SoundChannel_setLooping(lua_State *L)
{
    Binder binder(L);

    GGSoundChannel *soundChannel = static_cast<GGSoundChannel*>(binder.getInstance("SoundChannel", 1));

    soundChannel->setLooping(lua_toboolean2(L, 2));

    return 0;
}

int AudioBinder::SoundChannel_isLooping(lua_State *L)
{
    Binder binder(L);

    GGSoundChannel *soundChannel = static_cast<GGSoundChannel*>(binder.getInstance("SoundChannel", 1));

    lua_pushboolean(L, soundChannel->isLooping());

    return 1;
}

int AudioBinder::SoundChannel_setWorldPosition(lua_State *L)
{
    Binder binder(L);

    GGSoundChannel *soundChannel = static_cast<GGSoundChannel*>(binder.getInstance("SoundChannel", 1));

    soundChannel->setWorldPosition(luaL_optnumber(L, 2,0.0),luaL_optnumber(L, 3,0.0),luaL_optnumber(L, 4,0.0),
    		luaL_optnumber(L, 5,0.0),luaL_optnumber(L, 6,0.0),luaL_optnumber(L, 7,0.0));

    return 0;
}
