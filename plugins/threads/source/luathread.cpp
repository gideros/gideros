#include "luathread.h"
#include "gideros.h"
#include <cstdlib>
#include <chrono>
#include "lauxlib.h"


extern "C" {
  LUALIB_API int luaopen_lfs(lua_State *L);
  LUALIB_API int luaopen_socket_core(lua_State *L);
  LUALIB_API int luaopen_mime_core(lua_State *L);
  LUALIB_API int luaopen_cjson(lua_State *L);
  LUALIB_API int luaopen_cjson_safe(lua_State *L);
}

static void hook(lua_State* L, lua_Debug* ar);

std::string LuaThread::class_name = "Thread";

// avoid accidental name clash with fairly unique ID for instance pointer
static const char* k_this = "__fUID2543blah_";
static const char* k_runtime_error = "runtime error";

LuaThread::LuaThread(lua_State* main_lua_state, bool no_auto_debug_hook_for_termination) :
    m_main_state(main_lua_state),
    m_termination_requested(false),
    m_thread_status(LuaThread::Status::kNeedFunction),
    m_resume(false)
{
    // initialize our thread's Lua state
    m_thread_state = lua_newstate(LuaThread::alloc, nullptr);

    if (!no_auto_debug_hook_for_termination) {
        ThreadTimedLuaHook m_thread_timed_lua_hook(hook);
        m_thread_timed_lua_hook.setLua(m_thread_state);
    }

    luaL_openlibs(m_thread_state);

    // copy path and cpath to thread state, for require
    lua_getglobal(m_main_state, "package");
    lua_getglobal(m_thread_state, "package");
    lua_getfield(m_main_state, -1, "path");
    StateToState::copyStackEntry(m_main_state, m_thread_state, -1);
    lua_setfield(m_thread_state, -2, "path");
    lua_pop(m_main_state, 1);
    lua_getfield(m_main_state, -1, "cpath");
    StateToState::copyStackEntry(m_main_state, m_thread_state, -1);
    lua_setfield(m_thread_state, -2, "cpath");
    lua_pop(m_thread_state, 1);
    lua_pop(m_main_state, 2);

    // we send data from the thread state to this state, for retrieval
    // by the main state
    m_transfer_state = lua_newstate(LuaThread::alloc, nullptr);

    // store reference to this LuaThread instance
    lua_pushlightuserdata(m_thread_state, this);
    lua_setglobal(m_thread_state, k_this);

    // create Thread table with member functions to call from thread state
    lua_newtable(m_thread_state);
    lua_pushcfunction(m_thread_state, LuaThread::lua_thread_shouldTerminate);
    lua_setfield(m_thread_state, -2, "shouldTerminate");
    lua_pushcfunction(m_thread_state, LuaThread::lua_thread_sendData);
    lua_setfield(m_thread_state, -2, "sendData");
    lua_pushcfunction(m_thread_state, LuaThread::lua_thread_sleepFor);
    lua_setfield(m_thread_state, -2, "sleepFor");
    lua_pushcfunction(m_thread_state, LuaThread::lua_thread_yield);
    lua_setfield(m_thread_state, -2, "yield");
    lua_setglobal(m_thread_state, "Thread");

    lua_getglobal(m_thread_state, "package");
    lua_getfield(m_thread_state, -1, "preload");

    lua_pushcfunction(m_thread_state, luaopen_lfs);
    lua_setfield(m_thread_state, -2, "lfs");

    lua_pushcfunction(m_thread_state, luaopen_socket_core);
    lua_setfield(m_thread_state, -2, "socket.core");
    lua_pushcfunction(m_thread_state, luaopen_mime_core);
    lua_setfield(m_thread_state, -2, "mime.core");

    lua_pushcfunction(m_thread_state, luaopen_cjson);
    lua_setfield(m_thread_state, -2, "json");

    lua_pushcfunction(m_thread_state, luaopen_cjson_safe);
    lua_setfield(m_thread_state, -2, "json.safe");

    lua_pop(m_thread_state, 2);

    register_zlib(m_thread_state);
}

LuaThread::~LuaThread()
{
    // m_termination_requested set in lua_destroy - wait enough time for
    // lua_hook to call hook function to check for it and yield if set to
    // gracefully exit thread
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    lua_close(m_transfer_state);
    lua_close(m_thread_state);
}

int LuaThread::lua_getHardwareConcurrency(lua_State* L)
{
    lua_pushnumber(L, std::thread::hardware_concurrency());
    return 1;
}

int LuaThread::lua_thread_sleepFor(lua_State* L)
{
    int sleepy_time = lua_isnumber(L, -1) ? static_cast<int>(lua_tonumber(L, -1)) : 1;
    std::this_thread::sleep_for(std::chrono::microseconds(sleepy_time));
    return 0;
}

int LuaThread::lua_thread_shouldTerminate(lua_State* L)
{
    lua_getglobal(L, k_this);
    auto instance = static_cast<LuaThread*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    lua_pushboolean(L, instance->m_termination_requested);
    return 1;
}

int LuaThread::lua_thread_sendData(lua_State* L)
{
    lua_getglobal(L, k_this);
    auto instance = static_cast<LuaThread*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    instance->m_mutex.lock();
    StateToState::copyStackSlice(L, instance->m_transfer_state, -lua_gettop(L), -1);
    instance->m_mutex.unlock();
    return 0;
}

int LuaThread::lua_thread_yield(lua_State* L)
{
    lua_getglobal(L, k_this);
    auto instance = static_cast<LuaThread*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    instance->m_thread_status = LuaThread::Status::kSuspended;

    if (instance->hasTerminationBeenRequested()) lua_yield(L, 0);

    // resume gets results directly from thread state, since it's paused

    std::unique_lock<std::mutex> lock(instance->m_mutex);

    // thread pauses execution here, awaiting m_resume to be true
    instance->m_cv.wait(lock, [instance]{return instance->m_resume;});
    if (instance->m_resume)
        instance->m_thread_status = LuaThread::Status::kRunning;

    instance->m_resume = false; // reset for next time

    // return values possibly pushed onto stack by resume
    return lua_gettop(L);
}

int LuaThread::lua_requestTermination(lua_State* L)
{
    auto instance = static_cast<LuaThread*>(g_getInstance(L, LuaThread::class_name.c_str(), 1));
    instance->m_termination_requested = true;
    return 0;
}

bool LuaThread::hasTerminationBeenRequested()
{
    return m_termination_requested;
}

static void hook(lua_State* L, lua_Debug* ar)
{
    (void)ar;
    lua_getglobal(L, k_this);
    auto instance = static_cast<LuaThread*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (instance->hasTerminationBeenRequested()) {
        instance->m_thread_timed_lua_hook.stop();
        lua_yield(L, 0);
    }

    // always cancel hook, to be set again from ThreadTimesLuaHook instance
    // set MASK to 0 to disable hook
    lua_sethook(L, hook, 0, 0);

    // continue executing script / fall out of pcall
}

void LuaThread::worker(const std::shared_ptr<std::promise<int>>& promise)
{
    if (lua_pcall(m_thread_state, lua_gettop(m_thread_state) - 1, LUA_MULTRET, 0)) {
        lua_pushstring(m_thread_state, k_runtime_error);
        lua_insert(m_thread_state, -lua_gettop(m_thread_state));
    }

    // return number of return values/size of stack after call
    promise->set_value(lua_gettop(m_thread_state));
    m_termination_requested = false;
}

int LuaThread::execute(lua_State* L)
{
    if (m_thread_status != LuaThread::Status::kReady) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Warning: thread running, suspended or function not set");
        return 2;
    }

    int retvals = 0;
    int top = lua_gettop(L);
    lua_checkstack(m_thread_state, top + 2); // ensure enough space to grow stack
    if (top) {
        // push function arguments onto stack of thread state
        StateToState::copyStackSlice(L, m_thread_state, -top, -1);
    }
    m_promise = std::promise<int>();
    m_future = m_promise.get_future();
    // doesn't compile on MSVC unless promise is a shared_ptr
    auto sh = std::make_shared<std::promise<int>>(std::move(m_promise));
    std::thread m_thread(&LuaThread::worker, this, sh);
    m_thread_status = LuaThread::Status::kRunning;
    m_thread_timed_lua_hook.start();

    m_thread.detach();

    lua_pushboolean(L, true);
    lua_pushstring(L, "launched");
    retvals += 2;
    return retvals;
}

int LuaThread::lua_execute(lua_State* L)
{
    auto instance = static_cast<LuaThread*>(g_getInstance(L, LuaThread::class_name.c_str(), 1));
    lua_remove(L, 1);
    return instance->execute(L);
}

int LuaThread::lua_resume(lua_State* L)
{
    auto instance = static_cast<LuaThread*>(g_getInstance(L, LuaThread::class_name.c_str(), 1));
    lua_remove(L, 1);
    if (instance->m_thread_status != LuaThread::Status::kSuspended) {
        lua_settop(L, 1); // discard any arguments
        lua_pushboolean(L, false);
        lua_pushstring(L, "Warning: thread not suspended");
        return 2;
   }

    instance->m_mutex.lock();
    // store number of resume arguments so we know how big the bottom slice of the stack will be
    int num_args = lua_gettop(L);

    // return success
    lua_pushboolean(L, true);

    // get any arguments supplied by initial yield call
    int top = lua_gettop(instance->m_thread_state);
    if (top) {
        lua_checkstack(L, top);
        StateToState::copyStackSlice(instance->m_thread_state, L, -top, -1);
        lua_settop(instance->m_thread_state, 0);
    }

    // transfer arguments to thread state for yield call return values
    if (num_args) {
        lua_checkstack(instance->m_thread_state, num_args);
        StateToState::copyStackSlice(L, instance->m_thread_state, -lua_gettop(L), -top - 1);
    }

    // get rid of bottom args
    for (int i = 0; i < num_args; ++i) { lua_remove(L, -lua_gettop(L)); }

    instance->m_mutex.unlock();

    // continue execution of thread
    instance->m_resume = true;
    std::unique_lock<std::mutex> lock(instance->m_mutex);
    instance->m_cv.notify_one();

    return top + 1; // +1 for success bool
}

int LuaThread::lua_status(lua_State* L)
{
    auto instance = static_cast<LuaThread*>(g_getInstance(L, LuaThread::class_name.c_str(), 1));
    lua_remove(L, 1);

    switch (static_cast<LuaThread::Status>(instance->m_thread_status)) {
        case LuaThread::Status::kNeedFunction:
            lua_pushstring(L, "needs function");
            break;
        case LuaThread::Status::kReady:
            lua_pushstring(L, "ready");
            break;
        case LuaThread::Status::kRunning:
            lua_pushstring(L, "running");
            break;
        case LuaThread::Status::kSuspended:
            lua_pushstring(L, "suspended");
            break;
        case LuaThread::Status::kComplete:
            lua_pushstring(L, "complete");
            break;
    }

    return 1;
}

int LuaThread::setFunction(lua_State* L)
{
    if (m_thread_status == LuaThread::Status::kRunning || m_thread_status == LuaThread::Status::kSuspended) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Warning: can't set function, thread is running or suspended.");
        return 2;
    }
    int retvals = 0;
    lua_settop(m_thread_state, 0); // clear stack in thread Lua state

    if (lua_type(L, -1) == LUA_TFUNCTION) {
        StateToState::copyStackEntry(L, m_thread_state, -1);
        m_thread_status = LuaThread::Status::kReady;
        lua_pushboolean(L, true);
        ++retvals;
    } else {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Warning: function must be first argument to Thread:setFunction");
        retvals += 2;
    }
    return retvals;
}

int LuaThread::lua_setFunction(lua_State* L)
{
    auto instance = static_cast<LuaThread*>(g_getInstance(L, LuaThread::class_name.c_str(), 1));
    lua_remove(L, 1);
    return instance->setFunction(L);
}

int LuaThread::fetchData(lua_State* L)
{
    if (!(m_thread_status == LuaThread::Status::kRunning ||
        // can retrieve if suspended because transfer state is not used in yield/resume
        m_thread_status == LuaThread::Status::kSuspended)) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Warning: can't attempt to retrieve data unless thread is running or suspended.");
        return 2;
    }

    m_mutex.lock();
    int top = lua_gettop(m_transfer_state);
    if (top == 0) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "no data to get");
        m_mutex.unlock();
        return 2;
    }
    lua_pushboolean(L, true);
    StateToState::copyStackSlice(m_transfer_state, L, -top, -1);
    lua_pop(m_transfer_state, top);
    m_mutex.unlock();

    return top + 1;
}

int LuaThread::lua_fetchData(lua_State* L)
{
    auto instance = static_cast<LuaThread*>(g_getInstance(L, LuaThread::class_name.c_str(), 1));
    lua_remove(L, 1);
    return instance->fetchData(L);
}

int LuaThread::getResult(lua_State* L)
{
    if (m_thread_status != LuaThread::Status::kRunning) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Warning: Can't attempt to retreive result. ");
        return 2;
    }

    int retvals = 0;
    // get argument to function to set wait time (if any)
    int us = 0;
    if (lua_isnumber(L, -1)) {
        us = static_cast<int>(lua_tonumber(L, -1));
        lua_pop(L, 1);
    }
    us = us < 0 ? 0 : us;
    // waits for max 'us' microseconds for result
    auto res = m_future.wait_for(std::chrono::microseconds(us));

    if (res == std::future_status::ready)
    {
        int temp = m_future.get(); (void)temp; // invalidate future
        lua_pushboolean(L, true);
        ++retvals;
        const int top = lua_gettop(m_thread_state);
        StateToState::copyStackSlice(m_thread_state, L, -top, -1);
        retvals += top;
        lua_settop(m_thread_state, 0); // clear results from stack
        m_promise.~promise();
        m_thread_status = LuaThread::Status::kComplete;

        // check for flag we pushed onto thread stack if there was a pcall error...
        if (lua_gettop(L) >= 2 && lua_isstring(L, 2) && strcmp(lua_tostring(L, 2), k_runtime_error) == 0)
            lua_error(L); // ... and throw error, exiting with details in error message
    } else {
        lua_pushboolean(L, false);
        lua_pushstring(L, "running");
        retvals += 2;
    }
    return retvals;
}

int LuaThread::lua_getResult(lua_State* L)
{
    auto instance = static_cast<LuaThread*>(g_getInstance(L, LuaThread::class_name.c_str(), 1));
    lua_remove(L, 1);
    return instance->getResult(L);
}

int LuaThread::lua_create(lua_State *L)
{
    // if LuaThread.new(true) then don't use debug hook
    bool no_auto_debug_hook_for_termination = false;
    if (lua_isboolean(L, -1))
    {
        no_auto_debug_hook_for_termination = lua_toboolean(L, -1);
    }
    auto instance = new LuaThread(L, no_auto_debug_hook_for_termination);
    g_pushInstance(L, class_name.c_str(), instance);
    return 1;
}

int LuaThread::lua_destroy(lua_State *L)
{
    auto instance = *static_cast<LuaThread**>(lua_touserdata(L, 1));
    instance->m_termination_requested = true;
    instance->m_thread_timed_lua_hook.stop();
    // patience, Rodney... patience
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    delete instance;
    return 0;
}

// memory allocator for all our Lua states to use, because Gideros has a highly
// efficient memory pooling allocator for Lua which doesn't have to be thread aware
void* LuaThread::alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
    (void)ud;
    (void)osize;
    void *ret = nullptr;
    if (nsize == 0) {
       if (ptr)
          free(ptr);
    }
    else if (ptr == nullptr)
        ret = malloc(nsize);
    else
        ret = realloc(ptr, nsize);

    return ret;
}



