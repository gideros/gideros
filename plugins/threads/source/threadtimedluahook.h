/*

    timerhook.h:

        Set lua hook periodically from a timer in a thread. In the yield
        function in the thread running the Lua state, you must disable
        the hook with...

                lua_sethook(L, hook, 0, 0);

        ... each time it's called.


    Author: Paul Reilly

*/

#pragma once
#include "lua.hpp"
#include <atomic>


class ThreadTimedLuaHook
{
  public:
    // for function pointer to the lua hook function
    typedef void (*luahook_t)(lua_State *L, lua_Debug* ar);
    ThreadTimedLuaHook();
    ThreadTimedLuaHook(lua_State* L,
                       luahook_t hook_func);
    ThreadTimedLuaHook(const luahook_t hook_func);
    void worker();
    bool start();
    bool stop();
    bool isRunning();
    // interval is bounded (0.5ms -> 10s in 0.5ms steps), actual value set is returned
    int setInterval(int interval_in_microseconds);
    int getInterval();
    void setLua(lua_State *L);

  private:
    // state we will be setting the debug hook in
    lua_State* m_L;
    luahook_t m_hook_func;
    std::atomic<bool> m_is_running;
    std::atomic<bool> m_termination_requested;
    int m_interval_in_microseconds;
};
