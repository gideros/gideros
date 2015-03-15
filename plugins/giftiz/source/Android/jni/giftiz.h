#ifndef GIFTIZ_H
#define GIFTIZ_H

#include <gglobal.h>
#include <gevent.h>
#include <string>


typedef struct giftiz_Button
{
	int state;
} giftiz_Button;

enum
{
	GIFTIZ_BUTTON_STATE_CHANGE,
};


#ifdef __cplusplus
extern "C" {
#endif

G_API void giftiz_init();
G_API void giftiz_cleanup();

G_API bool giftiz_isAvailable();
G_API void giftiz_missionComplete();
G_API void giftiz_purchaseMade(float amount);
G_API int giftiz_getButtonState();
G_API void giftiz_buttonClicked();


G_API g_id giftiz_addCallback(gevent_Callback callback, void *udata);
G_API void giftiz_removeCallback(gevent_Callback callback, void *udata);
G_API void giftiz_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif