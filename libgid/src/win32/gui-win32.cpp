#include <gglobal.h>
#include <gevent.h>

extern "C" {

G_API void gui_init()
{
  //    s_manager = new UIManager;
}

G_API void gui_cleanup()
{
  //    delete s_manager;
  //    s_manager = NULL;
}

G_API g_id gui_createAlertDialog(const char *title,
                                 const char *message,
                                 const char *cancelButton,
                                 const char *button1,
                                 const char *button2,
                                 gevent_Callback callback,
                                 void *udata)
{
  //    return s_manager->createAlertDialog(title, message, cancelButton, button1, button2, callback, udata);
	return 0;
}

G_API g_id gui_createTextInputDialog(const char *title,
                                     const char *message,
                                     const char *text,
                                     const char *cancelButton,
                                     const char *button1,
                                     const char *button2,
                                     gevent_Callback callback,
                                     void *udata)
{
  //    return s_manager->createTextInputDialog(title, message, text, cancelButton, button1, button2, callback, udata);
	return 0;
}

G_API void gui_show(g_id gid)
{
  //    s_manager->show(gid);
}

G_API void gui_hide(g_id gid)
{
 //   s_manager->hide(gid);
}

G_API void gui_delete(g_id gid)
{
  //    s_manager->deleteWidget(gid);
}

G_API int gui_isVisible(g_id gid)
{
  //    return s_manager->isVisible(gid);
	return 0;
}

G_API void gui_setText(g_id gid, const char* text)
{
  //    s_manager->setText(gid, text);
}

G_API const char *gui_getText(g_id gid)
{
  //    return s_manager->getText(gid);
	return "gui_getText stub";
}

G_API void gui_setInputType(g_id gid, int inputType)
{
  //    s_manager->setInputType(gid, inputType);
}

G_API int gui_getInputType(g_id gid)
{
  //    return s_manager->getInputType(gid);
	return 0;
}

G_API void gui_setSecureInput(g_id gid, int secureInput)
{
  //    s_manager->setSecureInput(gid, secureInput);
}

G_API int gui_isSecureInput(g_id gid)
{
  //    return s_manager->isSecureInput(gid);
	return 0;
}

}
