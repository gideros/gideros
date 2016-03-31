#include <gglobal.h>
#include <gevent.h>
#include <map>
#include "resource.h"

class AlertDialog
{
public:
  string title,message;
  string cancelButton,button1,button2;
  void *udata;
  gevent_Callback callback;
}

static std::map<g_id, AlertDialog> map_;
static g_id mygid;

BOOL CALLBACK AlertDialogProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{

  if (iMsg==WM_INITDIALOG){

    SetWindowText(hwnd,map_[mygid].title.c_str());
    SetDlgItemText(hwnd,ID_MESSAGE,map_[mygid].message.c_str());

    SetDlgItemText(hwnd,ID_BUTTON1,map_[mygid].button1.c_str());
    SetDlgItemText(hwnd,ID_BUTTON2,map_[mygid].button2.c_str());
    SetDlgItemText(hwnd,ID_BUTTON3,map_[mygid].button3.c_str());

    return TRUE;
  }
  else if (iMsg==WM_COMMAND){
    if (LOWORD(wParam)==ID_BUTTON1){
      size_t size = sizeof(gui_AlertDialogCompleteEvent) + map_[mygid].button1.length() + 1;
      gui_AlertDialogCompleteEvent *event = (gui_AlertDialogCompleteEvent*)malloc(size);

      event->gid=mygid;
      event->buttonText=(char*)event+sizeof(gui_AlertDialogCompleteEvent);
      event->buttonIndex=1;
      strcpy((char*)event->buttonText, map_[gid].button1.c_str());
    }
    else if (LOWORD(wParam)==ID_BUTTON2){
      size_t size = sizeof(gui_AlertDialogCompleteEvent) + map_[mygid].button2.length() + 1;
      gui_AlertDialogCompleteEvent *event = (gui_AlertDialogCompleteEvent*)malloc(size);

      event->gid=mygid;
      event->buttonText=(char*)event+sizeof(gui_AlertDialogCompleteEvent);
      event->buttonIndex=2;
      strcpy((char*)event->buttonText, map_[gid].button2.c_str());
    }
    else {
      size_t size = sizeof(gui_AlertDialogCompleteEvent) + map_[mygid].cancelButton.length() + 1;
      gui_AlertDialogCompleteEvent *event = (gui_AlertDialogCompleteEvent*)malloc(size);

      event->gid=mygid;
      event->buttonText=(char*)event+sizeof(gui_AlertDialogCompleteEvent);
      event->buttonIndex=0;
      strcpy((char*)event->buttonText, map_[gid].cancelButton.c_str());
    }

    gevent_EnqueueEvent(mygid, map_[mygid].callback, GUI_ALERT_DIALOG_COMPLETE_EVENT, 
			event, 1, map_[mygid].udata);
    return TRUE;
  }
  else
    return FALSE;
}

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

  g_id gid = g_NextId();

  AlertDialog alertDialog;

  alertDialog.title=title;
  alertDialog.message=message;
  alertDialog.cancelButton;
  alertDialog.button1=button1;
  alertDialog.button2=button2;
  alertDialog.callback=callback;
  alertDialog.udata=udata;

  map_[gid]=alertDialog;
  
  return gid;
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

  std::map<g_id, AlertDialog>::iterator iter = map_.find(gid);

  if (iter == map_.end())
    throw std::runtime_error("invalid gid");
  
  iter->second.dialog->ShowAsync();

  DialogBox(hInst,MAKEINTRESOURCE(ID_ALERTDIALOG),hwnd,AlertDialogProc);

  mygid = gid;
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
