/* notes:
gevent_EnqueueEvent
------------------------
1. gid can never be zero. global systems also create their gid with g_NextId()
2. type enums can have collisions. therefore there is no function as gevent_removeEventsWithType.

gevent_AddCallback gevent_RemoveCallback gevent_RemoveCallbackWithGid
---------------------------------------------------------------------
1. These functions are only related to GEVENT_PRE_TICK_EVENT and GEVENT_POST_TICK_EVENT events and nothing else.

*/

#ifndef _GEVENT_H_
#define _GEVENT_H_

#include <gglobal.h>
#include <string.h>

enum
{
    GEVENT_PRE_TICK_EVENT,
    GEVENT_POST_TICK_EVENT,

    GAPPLICATION_OPEN_URL_EVENT,
    GAPPLICATION_START_EVENT,
    GAPPLICATION_EXIT_EVENT,
    GAPPLICATION_PAUSE_EVENT,
    GAPPLICATION_RESUME_EVENT,
    GAPPLICATION_BACKGROUND_EVENT,
    GAPPLICATION_FOREGROUND_EVENT,
    GAPPLICATION_MEMORY_LOW_EVENT,
    GAPPLICATION_ORIENTATION_CHANGE_EVENT,

    GINPUT_MOUSE_DOWN_EVENT,
    GINPUT_MOUSE_MOVE_EVENT,
    GINPUT_MOUSE_HOVER_EVENT,
    GINPUT_MOUSE_UP_EVENT,
	GINPUT_MOUSE_WHEEL_EVENT,

    GINPUT_TOUCH_BEGIN_EVENT,
    GINPUT_TOUCH_MOVE_EVENT,
    GINPUT_TOUCH_END_EVENT,
    GINPUT_TOUCH_CANCEL_EVENT,

    GINPUT_KEY_DOWN_EVENT,
    GINPUT_KEY_UP_EVENT,
    GINPUT_KEY_CHAR_EVENT,

    GGEOLOCATION_LOCATION_UPDATE_EVENT,
    GGEOLOCATION_HEADING_UPDATE_EVENT,
    GGEOLOCATION_ERROR_EVENT,

    GAUDIO_CHANNEL_COMPLETE_EVENT,
    GAUDIO_SYNC_POINT_EVENT,
};

#if 0
#define GHTTP_RESPONSE_EVENT 3
#define GHTTP_ERROR_EVENT 4
#define GHTTP_PROGRESS_EVENT 5

typedef struct ghttp_ResponseEvent
{

} ghttp_ResponseEvent;

typedef struct ghttp_ErrorEvent
{

} ghttp_ErrorEvent;

typedef struct ghttp_ProgressEvent
{

} ghttp_ProgressEvent;
#endif

typedef void(*gevent_Callback)(int type, void *event, void *udata);

#ifdef __cplusplus
extern "C" {
#endif

G_API void gevent_Init();
G_API void gevent_Cleanup();

G_API void gevent_Tick();

G_API void gevent_EnqueueEvent(g_id gid, gevent_Callback callback, int type, void *event, int free, void *udata);
G_API void gevent_RemoveEventsWithGid(g_id gid);

G_API g_id gevent_AddCallback(gevent_Callback callback, void *udata);
G_API void gevent_RemoveCallback(gevent_Callback callback, void *udata);
G_API void gevent_RemoveCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <vector>
class G_API gevent_CallbackList
{
public:
    gevent_CallbackList();
    ~gevent_CallbackList();

    g_id addCallback(gevent_Callback callback, void *udata);
    void removeCallback(gevent_Callback callback, void *udata);
    void removeCallbackWithGid(g_id gid);

    void dispatchEvent(int type, void *event);

private:
    struct Callback
    {
        g_id gid;
        gevent_Callback callback;
        void *udata;
    };

    struct CallbackCommand
    {
        int command;
        Callback callback;
    };

    struct equal_gid
    {
        equal_gid(g_id gid) : gid(gid) {}
        bool operator()(const Callback& c) const
        {
            return c.gid == gid;
        }
        g_id gid;
    };

    struct equal_callback
    {
        equal_callback(const Callback &c1) : c1(c1) {}
        bool operator()(const Callback &c2) const
        {
            return c1.callback == c2.callback && c1.udata == c2.udata;
        }
        Callback c1;
    };

    std::vector<Callback> callbacks_;
    std::vector<CallbackCommand> callbackCommands_;
};
#endif

#ifdef __cplusplus
extern "C" {
#endif
G_API void *gevent_CreateEventStruct1(size_t structSize,
                                      size_t offset1, const char *value1);

G_API void *gevent_CreateEventStruct2(size_t structSize,
                                      size_t offset1, const char *value1,
                                      size_t offset2, const char *value2);

G_API void *gevent_CreateEventStruct3(size_t structSize,
                                      size_t offset1, const char *value1,
                                      size_t offset2, const char *value2,
                                      size_t offset3, const char *value3);
#ifdef __cplusplus
}
#endif


#endif

