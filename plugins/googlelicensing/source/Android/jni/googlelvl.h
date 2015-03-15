#ifndef GOOGLELVL_H
#define GOOGLELVL_H

#include <gglobal.h>
#include <gevent.h>

enum
{
	GOOGLELVL_ALLOW_EVENT,
	GOOGLELVL_DISALLOW_EVENT,
	GOOGLELVL_RETRY_EVENT,
	GOOGLELVL_ERROR_EVENT,
	GOOGLELVL_DOWNLOAD_REQUIRED_EVENT,
	GOOGLELVL_DOWNLOAD_NOT_REQUIRED_EVENT,
	GOOGLELVL_DOWNLOAD_STATE_EVENT,
	GOOGLELVL_DOWNLOAD_PROGRESS_EVENT,
};

typedef struct ggooglelvl_SimpleEvent
{
	const char *error;
} ggooglelvl_SimpleEvent;

typedef struct ggooglelvl_StateEvent
{
	const char *state;
	const char *message;
} ggooglelvl_StateEvent;

typedef struct ggooglelvl_ProgressEvent
{
	float speed;
	long time;
	long progress;
	long total;
} ggooglelvl_ProgressEvent;

#ifdef __cplusplus
extern "C" {
#endif

G_API void ggooglelvl_init();
G_API void ggooglelvl_cleanup();

G_API void ggooglelvl_setKey(const char *key);

G_API void ggooglelvl_checkLicense();
G_API void ggooglelvl_checkExpansion();
G_API void ggooglelvl_cellularDownload(int use);

G_API g_id ggooglelvl_addCallback(gevent_Callback callback, void *udata);
G_API void ggooglelvl_removeCallback(gevent_Callback callback, void *udata);
G_API void ggooglelvl_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif