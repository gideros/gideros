#ifndef EXAMPLEPLUGIN_PLUGIN_BINDER
#define EXAMPLEPLUGIN_PLUGIN_BINDER

#include "gideros.h"
#include <gglobal.h>
#include <gevent.h>

enum
{
	GEXAMPLEPLUGIN_EVENT_STATE,
	GEXAMPLEPLUGIN_EVENT_WIFI,
};
typedef struct gexampleplugin_state
{
	int state;
	const char *description;
} gexampleplugin_state;

typedef struct gexampleplugin_wifi_permissions
{
	const char *change;
	const char *coarse;
	const char *fine;
} gexampleplugin_wifi_permissions;

typedef struct gexampleplugin_wifi_permission
{
	const char *permission;
} gexampleplugin_wifi_permission;

typedef struct gexampleplugin_wifi
{
	const char *action;
	int count;
	bool granted;
	gexampleplugin_wifi_permissions *permissions_all;
	int checked;
	gexampleplugin_wifi_permission *permissions_checked;
} gexampleplugin_wifi;

extern "C" {
G_API void gexampleplugin_dispatch(int type, void *event);
}
namespace exampleplugin {
	void init();
	void deinit();
	void start();
	void stop();
	bool test_binder();
}
#endif
