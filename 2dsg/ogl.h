#ifndef OGL_H_INCLUDED
#define OGL_H_INCLUDED

#include "Shaders.h"

#define PREMULTIPLIED_ALPHA 1

// remove any macros which will clash with C++ std::max, std::min
#ifdef WINSTORE
#undef min
#undef max
#endif

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif


void oglInitialize(unsigned int sw,unsigned int sh);
void oglCleanup();

//Renderer SYNC macros
#ifdef OCULUS
//#define OGL_THREADED_RENDERER
#endif
#ifdef QTWIN_THREADED_RENDERED
#define OGL_THREADED_RENDERER
#define gettid GetCurrentThreadId
#endif

#ifdef OGL_THREADED_RENDERER
#include <future>
#include <queue>
#include <unistd.h>
extern pid_t glThreadId;
extern std::queue<std::packaged_task<void()>> glTasks;
extern std::recursive_mutex glLock;
extern int glLockCount;
void glTaskWait(uint64_t ns);
void glRunTask(std::function<void()> f);
#define RENDER_START() glLock.lock();
#define RENDER_END() glLock.unlock();
#define RENDER_LOCK() glLock.lock(); glLockCount++;
#define RENDER_UNLOCK() glLockCount--; glLock.unlock();
#define RENDER_DO(f) /*if (glLockCount) throw std::invalid_argument("Already locked");*/ glRunTask(f)
#else
#define RENDER_START()
#define RENDER_END()
#define RENDER_LOCK()
#define RENDER_UNLOCK()
#define RENDER_DO(f) f()
#endif

#endif

