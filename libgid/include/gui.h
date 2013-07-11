#ifndef _GUI_H_
#define _GUI_H_

#include <gglobal.h>
#include <gevent.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GUI_ALERT_DIALOG_COMPLETE_EVENT 0
#define GUI_TEXT_INPUT_DIALOG_COMPLETE_EVENT 1

typedef struct gui_AlertDialogCompleteEvent
{
    g_id gid;
    int buttonIndex; // 0 for cancel, 1 for button1, 2 for button2
    const char *buttonText;
} gui_AlertDialogCompleteEvent;

typedef struct gui_TextInputDialogCompleteEvent
{
    g_id gid;
    const char *text;
    int buttonIndex; // 0 for cancel, 1 for button1, 2 for button2
    const char *buttonText;
} gui_TextInputDialogCompleteEvent;


// Text Input Types
#define GUI_TEXT_INPUT_DIALOG_TEXT 0
#define GUI_TEXT_INPUT_DIALOG_NUMBER 1
#define GUI_TEXT_INPUT_DIALOG_PHONE 2
#define GUI_TEXT_INPUT_DIALOG_EMAIL 3
#define GUI_TEXT_INPUT_DIALOG_URL 4

G_API void gui_init();
G_API void gui_cleanup();

G_API g_id gui_createAlertDialog(const char *title,
                                 const char *message,
                                 const char *cancelButton,
                                 const char *button1,
                                 const char *button2,
                                 gevent_Callback callback,
                                 void *udata);

G_API g_id gui_createTextInputDialog(const char *title,
                                     const char *message,
                                     const char *text,
                                     const char *cancelButton,
                                     const char *button1,
                                     const char *button2,
                                     gevent_Callback callback,
                                     void *udata);

// common functions for widgets/dialogs
G_API void gui_show(g_id gid);
G_API void gui_hide(g_id gid);
G_API int gui_isVisible(g_id gid);
G_API void gui_delete(g_id gid);

// functions for TextInputDialog
G_API void gui_setText(g_id gid, const char* text);
G_API const char *gui_getText(g_id gid);
G_API void gui_setInputType(g_id gid, int inputType);
G_API int gui_getInputType(g_id gid);
G_API void gui_setSecureInput(g_id gid, int secureInput);
G_API int gui_isSecureInput(g_id gid);

#ifdef __cplusplus
}
#endif

#endif
