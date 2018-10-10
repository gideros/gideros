/*

    LuaThread.h:

        Creates a Lua state that can run functions in a separate thread.

    Author: Paul Reilly

*/

#pragma once
#include "lua.hpp"
#include "macros.h"
#include <string>
#include <future>
#include "StateToState.h"
#include "threadtimedluahook.h"

void register_zlib(lua_State *L);

class LuaThread
{
  public:
    enum class Status {
        kNeedFunction,
        kReady,
        kRunning,
        kSuspended,
        kComplete
    };

    static std::string class_name; // set in .cpp

    LuaThread(lua_State* L);
    ~LuaThread();

    // worker thread that pcalls the supplied function in our Lua thread state
    void worker(const std::shared_ptr<std::promise<int> > &promise);

    /*    (lua_) functions that can be called from main Gideros Lua state    */

    // our thread needs a function to run, single argument (a function)
    int setFunction(lua_State* L);
    static int lua_setFunction(lua_State* L);

    // run function with this can supply optional arguments
    int execute(lua_State* L);
    static int lua_execute(lua_State* L);

    // if yield was called from function, resume with this
    static int lua_resume(lua_State* L);

    // returns (to Lua) status of the thread
    static int lua_status(lua_State* L);

    // fetches any data that has been passed by lua_thread_sendData
    int fetchData(lua_State* L);
    static int lua_fetchData(lua_State* L);

    // accepts optional value in microseconds to wait for thread to finish
    int getResult(lua_State* L);
    static int lua_getResult(lua_State* L);

    bool hasTerminationBeenRequested();
    // sets atomic variable for thread state to check via lua_thread_shouldTerminate
    static int lua_requestTermination(lua_State* L);

    // gets number of logical cores
    static int lua_getHardwareConcurrency(lua_State* L);


    /*    these only registered with our Lua thread state, to be called from    *
     *    within the supplied thread functions                                  */

    // argument is time in microseconds (defaults to 1)
    static int lua_thread_sleepFor(lua_State* L);

    static int lua_thread_yield(lua_State* L);

    // returns true if termination has been requested, for periodic
    // checking in infinite 'while' threads
    static int lua_thread_shouldTerminate(lua_State* L);

    // for returning data to main state, without suspending/yielding
    static int lua_thread_sendData(lua_State* L);

  private:
    lua_State* m_thread_state;
    lua_State* m_transfer_state;
    lua_State* m_main_state;

    std::promise<int> m_promise;
    std::future<int> m_future;
    std::unique_ptr<std::thread> m_thread;
    std::atomic<bool> m_termination_requested;
    std::atomic<LuaThread::Status> m_thread_status;
    // for thread yield and resume
    std::condition_variable m_cv;
    bool m_resume;
    std::mutex m_mutex;

  public:
    ThreadTimedLuaHook m_thread_timed_lua_hook;
    static void* alloc(void *ud, void *ptr, size_t osize, size_t nsize);
    static int lua_create(lua_State *L);
    static int lua_destroy(lua_State *L);
};

