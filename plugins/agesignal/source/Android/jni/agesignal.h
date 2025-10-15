#ifndef GOOGLELVL_H
#define GOOGLELVL_H

#include <gglobal.h>
#include <gevent.h>

enum
{
	AGESIGNAL_AGE_SIGNALS_ENV,
};

typedef struct gagesignals_AgeSignalsEvent
{
	const char *installId;
	const char *status;
	int64_t approvalDate;
	int ageLower;
	int ageUpper;
} gagesignals_AgeSignalsEvent;

#ifdef __cplusplus
extern "C" {
#endif

G_API void gagesignal_init();
G_API void gagesignal_cleanup();

G_API void gagesignal_checkAgeSignals();

G_API g_id gagesignal_addCallback(gevent_Callback callback, void *udata);
G_API void gagesignal_removeCallback(gevent_Callback callback, void *udata);
G_API void gagesignal_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif
