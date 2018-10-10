#include "threadtimedluahook.h"
#include <thread>


ThreadTimedLuaHook::ThreadTimedLuaHook(
                     lua_State* L,
                     const luahook_t hook_func
                     )
                   : m_L(L),
                     m_hook_func(hook_func),
                     m_is_running(false),
                     m_termination_requested(false),
                     m_interval_in_microseconds(1000)
{
    //
}

ThreadTimedLuaHook::ThreadTimedLuaHook(
                    const luahook_t hook_func)
                  : m_hook_func(hook_func),
                    m_is_running(false),
                    m_termination_requested(false),
                    m_interval_in_microseconds(1000)
{
    //
}

void ThreadTimedLuaHook::worker()
{
    // check for exit every 500 microseconds
    int segments, count;
    segments = count = static_cast<int>(m_interval_in_microseconds / 500);

    while (!m_termination_requested) {
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        --count;
        if (count <= 0) {
            // set hook for 1 instruction later, then it's cancelled in the hook function
            lua_sethook(m_L, m_hook_func, LUA_MASKCOUNT, 1);
            count = segments;
        }
    }
    m_termination_requested = m_is_running = false;
}

bool ThreadTimedLuaHook::start()
{
    if (m_is_running || m_L == nullptr) return false;
    std::thread t(&ThreadTimedLuaHook::worker, this);
    m_is_running = true;
    t.detach();
    return true;
}

bool ThreadTimedLuaHook::stop()
{
    if (!m_is_running) return false;
    m_termination_requested = true;
    return true;
}

bool ThreadTimedLuaHook::isRunning()
    { return m_is_running; }

int ThreadTimedLuaHook::setInterval(int interval_in_microseconds)
{
    // bound to range of 0.5ms -> 10s
    if (interval_in_microseconds < 500) interval_in_microseconds = 500;
    if (interval_in_microseconds > 10000000) interval_in_microseconds = 10000000;
    // round down to a 0.5ms step
    interval_in_microseconds -= interval_in_microseconds % 500;

    m_interval_in_microseconds = interval_in_microseconds;
    return interval_in_microseconds;
}

int ThreadTimedLuaHook::getInterval()
    { return m_interval_in_microseconds; }

void ThreadTimedLuaHook::setLua(lua_State *L)
    { m_L = L; }
