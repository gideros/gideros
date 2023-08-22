#include <gapplication.h>
#include <gapplication-linux.h>
#include <GLFW/glfw3.h>
#include <cstdio>

class GGApplicationManager
{
    
public:
    GGApplicationManager()
    {
        gid_ = g_NextId();
    }
    
    ~GGApplicationManager()
    {
        gevent_RemoveEventsWithGid(gid_);
    }
	
	int getScreenDensity()
    {   
	  GLFWmonitor *m=glfwGetPrimaryMonitor();
	  const GLFWvidmode *vm=glfwGetVideoMode(m);
	  int wm,hm;
	  glfwGetMonitorPhysicalSize(m,&wm,&hm);
	  double dpi=((double)vm->width)*25.4/wm;
	  float sx,sy;
	  glfwGetMonitorContentScale(m,&sx,&sy);
      return dpi;
    }

	void exit()
	{
		//exit(0);
	}

    g_id addCallback(gevent_Callback callback, void *udata)
    {
        return callbackList_.addCallback(callback, udata);
    }
    
    void removeCallback(gevent_Callback callback, void *udata)
    {
        callbackList_.removeCallback(callback, udata);
    }
    
    void removeCallbackWithGid(g_id gid)
    {
        callbackList_.removeCallbackWithGid(gid);
    }
    
    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<GGApplicationManager*>(udata)->callbackList_.dispatchEvent(type, event);
    }
    
    void enqueueEvent(int type, void *event, int free)
    {
        gevent_EnqueueEvent(gid_, callback_s, type, event, free, this);
    }
    
private:
    gevent_CallbackList callbackList_;
    g_id gid_;
};


static GGApplicationManager *s_manager = NULL;

extern "C" {

void gapplication_init()
{
    s_manager = new GGApplicationManager;
}

void gapplication_cleanup()
{
    delete s_manager;
    s_manager = NULL;
}

g_id gapplication_addCallback(gevent_Callback callback, void *udata)
{
    return s_manager->addCallback(callback, udata);
}

void gapplication_removeCallback(gevent_Callback callback, void *udata)
{
    s_manager->removeCallback(callback, udata);
}

void gapplication_removeCallbackWithGid(g_id gid)
{
    s_manager->removeCallbackWithGid(gid);
}

void gapplication_exit()
{
	s_manager->exit();
}

int gapplication_getScreenDensity(int *ldpi){
	return s_manager->getScreenDensity();
}

void gapplication_enqueueEvent(int type, void *event, int free)
{
    s_manager->enqueueEvent(type, event, free);
}

}
