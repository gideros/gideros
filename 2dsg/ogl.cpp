#include "ogl.h"
#include "glog.h"
#include <string.h>
#include "gtexture.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#ifdef WINSTORE
#include "dx11Shaders.h"
#elif defined(QT_CORE_LIB)
#include "gl2Shaders.h"
#elif defined(TARGET_OS_MAC)
extern ShaderEngine *createMetalShaderEngine(int sw,int sh);
#else
#include "gl2Shaders.h"
#endif
#include "screen.h"

#ifdef OGL_THREADED_RENDERER
//#define DEBUG_TASKS
pid_t glThreadId=0;
std::queue<std::packaged_task<void()>> glTasks;
static std::mutex glTaskLock;
static std::condition_variable glTaskCv;
void glRunTask(std::function<void()> f)
{
	if ((glThreadId==0)||(glThreadId==gettid()))
	{
		f();
	}
	else {
#ifdef DEBUG_TASKS
		glog_i("STASK");
	    const auto start{std::chrono::high_resolution_clock::now()};
#endif
		std::packaged_task<void()> task(f);
		std::future<void> res=task.get_future();
		glTasks.push(std::move(task));
		glTaskCv.notify_one();
		res.get();
#ifdef DEBUG_TASKS
	    const auto end{std::chrono::high_resolution_clock::now()};
		glog_i("DTASK:%llu us",std::chrono::duration_cast<std::chrono::microseconds>(end - start));
#endif
	}
}
void glTaskWait(uint64_t ns) {
#ifdef DEBUG_TASKS
	glog_i("TTASK:%llu us",ns/1000);
	const auto mstart{std::chrono::high_resolution_clock::now()};
#endif
    const auto lend{std::chrono::high_resolution_clock::now()+std::chrono::nanoseconds(ns)};

    while (true) {
		if (!glTasks.empty()) {
#ifdef DEBUG_TASKS
			glog_i("RTASK");
			const auto start{std::chrono::high_resolution_clock::now()};
#endif
			glTasks.front()();
#ifdef DEBUG_TASKS
			const auto end{std::chrono::high_resolution_clock::now()};
			glog_i("FTASK:%llu us",std::chrono::duration_cast<std::chrono::microseconds>(end - start));
#endif
			glTasks.pop();
		}
        if (std::chrono::high_resolution_clock::now()>lend) break;
        if (glTasks.empty()) {
			std::unique_lock<std::mutex> lk(glTaskLock);
			glTaskCv.wait_until(lk,lend);
		}
    }
#ifdef DEBUG_TASKS
	const auto mend{std::chrono::high_resolution_clock::now()};
	glog_i("WTASK:%llu us",std::chrono::duration_cast<std::chrono::microseconds>(mend - mstart));
#endif
}
int glLockCount=0;
std::recursive_mutex glLock;
#endif

void oglInitialize(unsigned int sw, unsigned int sh) {
	if (ShaderEngine::Engine)
		return;
#ifdef WINSTORE
	ShaderEngine::Engine = new dx11ShaderEngine(sw, sh);
#elif defined(QT_CORE_LIB)
    ShaderEngine::Engine = new ogl2ShaderEngine(sw, sh);
#elif defined(TARGET_OS_MAC)
    ShaderEngine::Engine=createMetalShaderEngine(sw,sh);
#else
	ShaderEngine::Engine = new ogl2ShaderEngine(sw, sh);
#endif
	ShaderEngine::Engine->setVBOThreshold(10,10);
	gtexture_set_engine(ShaderEngine::Engine);
	gtexture_set_screenmanager(ScreenManager::manager);
}

void oglCleanup() {
	if (ShaderEngine::Engine) {
		gtexture_set_screenmanager(NULL);
		gtexture_set_engine(NULL);
		delete ShaderEngine::Engine;
		ShaderEngine::Engine = NULL;
	}
}
