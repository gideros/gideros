#include <gevent.h>
#include <deque>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <stddef.h>
#include "gapplication.h"

namespace gevent {

class EventManager
{
public:
    EventManager()
    {
        pthread_mutex_init(&mutex_, NULL);
    }

    ~EventManager()
    {
        pthread_mutex_destroy(&mutex_);
    }

    void tick();

    void enqueueEvent(g_id gid, gevent_Callback callback, int type, void *event, int free, void *udata, bool merge);
    void removeEventsWithGid(g_id gid);
    void removeEventsWithType(int type);

private:
    struct EventQueueElement
    {
        g_id gid;
        gevent_Callback callback;
        int type;
        void *event;
        int free;
        void *udata;
        int merge;
    };

    template <typename T>
    struct equal_gid
    {
        equal_gid(g_id gid) : gid(gid) {}
        bool operator()(const T& t) const
        {
            return t.gid == gid;
        }
        g_id gid;
    };

    template <typename T>
    struct equal_type
    {
        equal_type(int type) : type(type) {}
        bool operator()(const T& t) const
        {
            return t.type == type;
        }
        int type;
    };

    std::deque<EventQueueElement> eventQueue_;
    pthread_mutex_t mutex_;

    struct scope_exit_free
    {
        scope_exit_free(bool free, void *ptr) : free(free), ptr(ptr)
        {

        }
        ~scope_exit_free()
        {
            if (free)
                ::free(ptr);
        }
        bool free;
        void *ptr;
    };

public:
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

private:
    gevent_CallbackList callbackList_;
};

void EventManager::tick()
{
    callbackList_.dispatchEvent(GEVENT_PRE_TICK_EVENT, NULL);

    pthread_mutex_lock(&mutex_);
    while (true)
    {

        if (eventQueue_.empty()) break;

        EventQueueElement element = eventQueue_.front();
        eventQueue_.pop_front();

    	bool skip=false;
        if (element.merge&&!eventQueue_.empty())
        {
        	size_t qs=eventQueue_.size();
        	size_t i=0;
        	while (i<qs) {
        		if (!eventQueue_[i].merge) break;
        		if ((eventQueue_[i].gid==element.gid)&&(eventQueue_[i].callback==element.callback)&&(eventQueue_[i].type==element.type))
        		{
        			skip=true;
        			break;
        		}
        		i++;
        	}
        }


    	{
            scope_exit_free f(element.free, element.event);
    		if (!skip) {
    			if (!element.merge)
    				pthread_mutex_unlock(&mutex_);

				if (element.callback)
					element.callback(element.type, element.event, element.udata);

    			if (!element.merge)
    				pthread_mutex_lock(&mutex_);
    		}
    	}
    }
    pthread_mutex_unlock(&mutex_);

    callbackList_.dispatchEvent(GEVENT_POST_TICK_EVENT, NULL);
}

void EventManager::enqueueEvent(g_id gid, gevent_Callback callback, int type, void *event, int free, void *udata, bool merge)
{
    EventQueueElement element;
    element.gid = gid;
    element.callback = callback;
    element.type = type;
    element.event = event;
    element.free = free;
    element.udata = udata;
    element.merge = merge;

    pthread_mutex_lock(&mutex_);
    eventQueue_.push_back(element);
    pthread_mutex_unlock(&mutex_);
}

void EventManager::removeEventsWithGid(g_id gid)
{
    pthread_mutex_lock(&mutex_);

    std::deque<EventQueueElement>::iterator first = eventQueue_.begin();
    std::deque<EventQueueElement>::iterator last = eventQueue_.end();
    std::deque<EventQueueElement>::iterator it;

    for (it=first; it != last; ++it)
        if ((it->free)&&(it->gid==gid))
            free(it->event);

    eventQueue_.erase(std::remove_if(first, last, equal_gid<EventQueueElement>(gid)),last);

    pthread_mutex_unlock(&mutex_);
}

void EventManager::removeEventsWithType(int type)
{
    pthread_mutex_lock(&mutex_);

    std::deque<EventQueueElement>::iterator first = eventQueue_.begin();
    std::deque<EventQueueElement>::iterator last = eventQueue_.end();
    std::deque<EventQueueElement>::iterator it;
    
    for (it=first; it != last; ++it)
        if ((it->free)&&(it->type==type))
            free(it->event);
    
    eventQueue_.erase(std::remove_if(first, last, equal_type<EventQueueElement>(type)),last);

    pthread_mutex_unlock(&mutex_);
}

}

using namespace gevent;

static EventManager *s_manager = NULL;

extern "C" {

void gevent_Init()
{
    s_manager = new EventManager;
}

void gevent_Cleanup()
{
    delete s_manager;
    s_manager = NULL;
}

void gevent_Tick()
{
    s_manager->tick();
}

static void (*_flusher)()=NULL;
static bool mergeEvents=false;
void gevent_SetFlusher(void (*flusher)())
{
	_flusher=flusher;
}

void gevent_AllowEventMerge(int allow)
{
	mergeEvents=allow;
}

void gevent_Flush()
{
	if (_flusher)
		_flusher();
}

void gevent_EnqueueEvent(g_id gid, gevent_Callback callback, int type, void *event, int free, void *udata)
{
    s_manager->enqueueEvent(gid, callback, type, event, free, udata, false);
}

void gevent_MergeEvent(g_id gid, gevent_Callback callback, int type, void *event, int free, void *udata)
{
    s_manager->enqueueEvent(gid, callback, type, event, free, udata, mergeEvents);
}

void gevent_RemoveEventsWithGid(g_id gid)
{
    s_manager->removeEventsWithGid(gid);
}

g_id gevent_AddCallback(gevent_Callback callback, void *udata)
{
    return s_manager->addCallback(callback, udata);
}

void gevent_RemoveCallback(gevent_Callback callback, void *udata)
{
    s_manager->removeCallback(callback, udata);
}

void gevent_RemoveCallbackWithGid(g_id gid)
{
    s_manager->removeCallbackWithGid(gid);
}

}


gevent_CallbackList::gevent_CallbackList()
{
}

gevent_CallbackList::~gevent_CallbackList()
{

}

g_id gevent_CallbackList::addCallback(gevent_Callback callback, void *udata)
{
    CallbackCommand c;
    c.command = 0;
    c.callback.gid = g_NextId();
    c.callback.callback = callback;
    c.callback.udata = udata;

    callbackCommands_.push_back(c);

    return c.callback.gid;
}

void gevent_CallbackList::removeCallback(gevent_Callback callback, void *udata)
{
    CallbackCommand c;
    c.command = 1;
    c.callback.callback = callback;
    c.callback.udata = udata;

    callbackCommands_.push_back(c);
}

void gevent_CallbackList::removeCallbackWithGid(g_id gid)
{
    CallbackCommand c;
    c.command = 2;
    c.callback.gid = gid;

    callbackCommands_.push_back(c);
}

void gevent_CallbackList::dispatchEvent(int type, void *event)
{
    for (size_t i = 0; i < callbackCommands_.size(); ++i)
    {
        const CallbackCommand &c = callbackCommands_[i];

        if (c.command == 0)
        {
            callbacks_.push_back(c.callback);
        }
        else if (c.command == 1)
        {
            std::vector<Callback> &v = callbacks_;
            v.erase(std::remove_if(v.begin(), v.end(), equal_callback(c.callback)), v.end());
        }
        else if (c.command == 2)
        {
            std::vector<Callback> &v = callbacks_;
            v.erase(std::remove_if(v.begin(), v.end(), equal_gid(c.callback.gid)), v.end());
        }
    }
    callbackCommands_.clear();

    for (size_t i = 0; i < callbacks_.size(); ++i)
    {
        if (callbacks_[i].callback)
            callbacks_[i].callback(type, event, callbacks_[i].udata);
    }
}

extern "C" {
void *gevent_CreateEventStruct1(size_t structSize,
                                size_t offset1, const char *value1)
{
    size_t size1 = value1 ? (strlen(value1) + 1) : 0;

    void *result = malloc(structSize + size1);

    char **field1 = (char**)((char*)result + offset1);

    *field1 = value1 ? strcpy((char*)result + structSize, value1) : NULL;

    return result;
}


void *gevent_CreateEventStruct2(size_t structSize,
                                size_t offset1, const char *value1,
                                size_t offset2, const char *value2)
{
    size_t size1 = value1 ? (strlen(value1) + 1) : 0;
    size_t size2 = value2 ? (strlen(value2) + 1) : 0;

    void *result = malloc(structSize + size1 + size2);

    char **field1 = (char**)((char*)result + offset1);
    char **field2 = (char**)((char*)result + offset2);

    *field1 = value1 ? strcpy((char*)result + structSize,         value1) : NULL;
    *field2 = value2 ? strcpy((char*)result + structSize + size1, value2) : NULL;

    return result;
}


void *gevent_CreateEventStruct3(size_t structSize,
                                size_t offset1, const char *value1,
                                size_t offset2, const char *value2,
                                size_t offset3, const char *value3)
{
    size_t size1 = value1 ? (strlen(value1) + 1) : 0;
    size_t size2 = value2 ? (strlen(value2) + 1) : 0;
    size_t size3 = value3 ? (strlen(value3) + 1) : 0;

    void *result = malloc(structSize + size1 + size2 + size3);

    char **field1 = (char**)((char*)result + offset1);
    char **field2 = (char**)((char*)result + offset2);
    char **field3 = (char**)((char*)result + offset3);

    *field1 = value1 ? strcpy((char*)result + structSize,                 value1) : NULL;
    *field2 = value2 ? strcpy((char*)result + structSize + size1,         value2) : NULL;
    *field3 = value3 ? strcpy((char*)result + structSize + size1 + size2, value3) : NULL;

    return result;
}

}

void gevent_EnqueuePermissionsResult(std::map<std::string,int> &perms)
{

	int pCount=0;
	int pSize=0;

	for (auto it=perms.begin();it!=perms.end();it++) {
		pCount++;
		pSize+=sizeof(char *)+sizeof(int)+it->first.size()+1;
	}
	gapplication_PermissionEvent *event = (gapplication_PermissionEvent*)malloc(sizeof(gapplication_PermissionEvent) + pSize);
	event->count=pCount;
	event->status=(int *)(((char *)event)+sizeof(gapplication_PermissionEvent));
	event->perms=(const char **)(((char *)(event->status))+sizeof(int)*pCount);
	char *data=(((char *)(event->perms))+sizeof(char *)*pCount);

	pCount=0;
	for (auto it=perms.begin();it!=perms.end();it++) {
		event->status[pCount]=it->second;
		event->perms[pCount]=data;
		strcpy(data,it->first.c_str());
		data+=it->first.size()+1;
		pCount++;
	}

    gapplication_enqueueEvent(GAPPLICATION_PERMISSION_EVENT, event, 1);
}
