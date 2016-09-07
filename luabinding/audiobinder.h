#ifndef AUDIOBINDER_H
#define AUDIOBINDER_H

#include "binder.h"

class AudioBinder
{
public:
    AudioBinder(lua_State *L);

private:
    static int Sound_create(lua_State *L);
    static int Sound_destruct(lua_State *L);
    static int Sound_getLength(lua_State *L);
    static int Sound_play(lua_State *L);
    static int Sound_setListenerPosition(lua_State *L);

    static int SoundChannel_destruct(lua_State *L);
    static int SoundChannel_stop(lua_State *L);
    static int SoundChannel_setPosition(lua_State *L);
    static int SoundChannel_getPosition(lua_State *L);
    static int SoundChannel_setVolume(lua_State *L);
    static int SoundChannel_getVolume(lua_State *L);
    static int SoundChannel_setPitch(lua_State *L);
    static int SoundChannel_getPitch(lua_State *L);
    static int SoundChannel_isPlaying(lua_State *L);
    static int SoundChannel_setPaused(lua_State *L);
    static int SoundChannel_isPaused(lua_State *L);
    static int SoundChannel_setLooping(lua_State *L);
    static int SoundChannel_isLooping(lua_State *L);
    static int SoundChannel_setWorldPosition(lua_State *L);
};


#endif
