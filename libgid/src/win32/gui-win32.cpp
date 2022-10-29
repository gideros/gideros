#include <windows.h>
#include <gglobal.h>
#include <gevent.h>
#include <gui.h>
#include <map>
#include <string>
#include <stdio.h>
#include "resource.h"

using namespace std;

static std::wstring ws(const char *str)
{
    if (!str) return std::wstring();
    int sl=strlen(str);
    int sz = MultiByteToWideChar(CP_UTF8, 0, str, sl, 0, 0);
    std::wstring res(sz, 0);
    MultiByteToWideChar(CP_UTF8, 0, str, sl, &res[0], sz);
    return res;
}

static std::string us(const wchar_t *str)
{
    if (!str) return std::string();
    int sl=wcslen(str);
    int sz = WideCharToMultiByte(CP_UTF8, 0, str, sl, 0, 0,NULL,NULL);
    std::string res(sz, 0);
    WideCharToMultiByte(CP_UTF8, 0, str, sl, &res[0], sz,NULL,NULL);
    return res;
}
class Win32Dialog
{
public:
  string title,message;
  string cancelButton,button1,button2;
  string text;
  void *udata;
  gevent_Callback callback;
  int type,nbutton;
};

static HINSTANCE hInstcopy;
static HWND hwndcopy;

static std::map<g_id, Win32Dialog> map_;
static g_id mygid;

// ######################################################################

INT_PTR CALLBACK TextInputDialogProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
  wchar_t buffer[1024];
  gui_TextInputDialogCompleteEvent *event;

  if (iMsg==WM_INITDIALOG){
	std:wstring wt=ws(map_[mygid].title.c_str());
    SetWindowText(hwnd,wt.c_str());
    wt=ws(map_[mygid].message.c_str());
    SetDlgItemText(hwnd,TID_MESSAGE,wt.c_str());

    if (map_[mygid].nbutton>=2) {
    	wt=ws(map_[mygid].button1.c_str());
    	SetDlgItemText(hwnd,TID_BUTTON1,wt.c_str());
    }
    else
      ShowWindow(GetDlgItem(hwnd,TID_BUTTON1),SW_HIDE);

    if (map_[mygid].nbutton==3) {
    	wt=ws(map_[mygid].button2.c_str());
      SetDlgItemText(hwnd,TID_BUTTON2,wt.c_str());
    }
    else
      ShowWindow(GetDlgItem(hwnd,TID_BUTTON2),SW_HIDE);

    wt=ws(map_[mygid].cancelButton.c_str());
    SetDlgItemText(hwnd,TID_BUTTON3,wt.c_str());

    wt=ws(map_[mygid].text.c_str());
    SetDlgItemText(hwnd,TID_EDIT,wt.c_str());

    return TRUE;
  }
  else if (iMsg==WM_COMMAND){
    if (LOWORD(wParam)==TID_BUTTON1){
      GetDlgItemText(hwnd,TID_EDIT,buffer,1024);
      std::string bt=us(buffer);

      size_t size = sizeof(gui_TextInputDialogCompleteEvent) + bt.size() + 1
	+ map_[mygid].button1.length() + 1;

      event = (gui_TextInputDialogCompleteEvent*)malloc(size);

      event->gid=mygid;
      event->buttonIndex=1;

      event->text =     (char*)event + sizeof(gui_TextInputDialogCompleteEvent);
      event->buttonText=(char*)event + sizeof(gui_TextInputDialogCompleteEvent)
  	               +bt.size()+1;

      strcpy((char*)event->buttonText, map_[mygid].button1.c_str());
      strcpy((char*)event->text,bt.c_str());

      EndDialog(hwnd,LOWORD(wParam));
      gevent_EnqueueEvent(mygid, map_[mygid].callback, GUI_TEXT_INPUT_DIALOG_COMPLETE_EVENT, 
			  event, 1, map_[mygid].udata);
      return TRUE;
    }
    else if (LOWORD(wParam)==TID_BUTTON2){
      GetDlgItemText(hwnd,TID_EDIT,buffer,1024);
      std::string bt=us(buffer);

      size_t size = sizeof(gui_TextInputDialogCompleteEvent) + bt.size() + 1
	+ map_[mygid].button2.length() + 1;

      event = (gui_TextInputDialogCompleteEvent*)malloc(size);

      event->gid=mygid;
      event->buttonIndex=2;

      event->text =     (char*)event + sizeof(gui_TextInputDialogCompleteEvent);
      event->buttonText=(char*)event + sizeof(gui_TextInputDialogCompleteEvent)
  	               +bt.size()+1;

      strcpy((char*)event->buttonText, map_[mygid].button2.c_str());
      strcpy((char*)event->text,bt.c_str());

      EndDialog(hwnd,LOWORD(wParam));
      gevent_EnqueueEvent(mygid, map_[mygid].callback, GUI_TEXT_INPUT_DIALOG_COMPLETE_EVENT, 
			  event, 1, map_[mygid].udata);
      return TRUE;
    }
    else if (LOWORD(wParam)==TID_BUTTON3){
      GetDlgItemText(hwnd,TID_EDIT,buffer,1024);
      std::string bt=us(buffer);

      size_t size = sizeof(gui_TextInputDialogCompleteEvent) + bt.size() + 1
	+ map_[mygid].cancelButton.length() + 1;

      event = (gui_TextInputDialogCompleteEvent*)malloc(size);

      event->gid=mygid;
      event->buttonIndex=0;

      event->text =     (char*)event + sizeof(gui_TextInputDialogCompleteEvent);
      event->buttonText=(char*)event + sizeof(gui_TextInputDialogCompleteEvent)
  	               +bt.size()+1;

      strcpy((char*)event->buttonText, map_[mygid].cancelButton.c_str());
      strcpy((char*)event->text,bt.c_str());

      EndDialog(hwnd,LOWORD(wParam));
      gevent_EnqueueEvent(mygid, map_[mygid].callback, GUI_TEXT_INPUT_DIALOG_COMPLETE_EVENT, 
			  event, 1, map_[mygid].udata);
      return TRUE;
    }
    return FALSE;
  }
  else
    return FALSE;
}

// ######################################################################

INT_PTR CALLBACK AlertDialogProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
  gui_AlertDialogCompleteEvent *event;

  if (iMsg==WM_INITDIALOG){
		std:wstring wt=ws(map_[mygid].title.c_str());
	    SetWindowText(hwnd,wt.c_str());
	    wt=ws(map_[mygid].message.c_str());
	    SetDlgItemText(hwnd,ID_MESSAGE,wt.c_str());

	    if (map_[mygid].nbutton>=2) {
	    	wt=ws(map_[mygid].button1.c_str());
	    	SetDlgItemText(hwnd,ID_BUTTON1,wt.c_str());
	    }
	    else
	      ShowWindow(GetDlgItem(hwnd,ID_BUTTON1),SW_HIDE);

	    if (map_[mygid].nbutton==3) {
	    	wt=ws(map_[mygid].button2.c_str());
	      SetDlgItemText(hwnd,ID_BUTTON2,wt.c_str());
	    }
	    else
	      ShowWindow(GetDlgItem(hwnd,ID_BUTTON2),SW_HIDE);

	    wt=ws(map_[mygid].cancelButton.c_str());
	    SetDlgItemText(hwnd,ID_BUTTON3,wt.c_str());

    return TRUE;
  }
  else if (iMsg==WM_COMMAND){
    if (LOWORD(wParam)==ID_BUTTON1){
      size_t size = sizeof(gui_AlertDialogCompleteEvent) + map_[mygid].button1.length() + 1;
      event = (gui_AlertDialogCompleteEvent*)malloc(size);

      event->gid=mygid;
      event->buttonText=(char*)event+sizeof(gui_AlertDialogCompleteEvent);
      event->buttonIndex=1;
      strcpy((char*)event->buttonText, map_[mygid].button1.c_str());

      gevent_EnqueueEvent(mygid, map_[mygid].callback, GUI_ALERT_DIALOG_COMPLETE_EVENT, 
			  event, 1, map_[mygid].udata);
      
      EndDialog(hwnd,LOWORD(wParam));
      return TRUE;
    }
    else if (LOWORD(wParam)==ID_BUTTON2){
      size_t size = sizeof(gui_AlertDialogCompleteEvent) + map_[mygid].button2.length() + 1;
      event = (gui_AlertDialogCompleteEvent*)malloc(size);

      event->gid=mygid;
      event->buttonText=(char*)event+sizeof(gui_AlertDialogCompleteEvent);
      event->buttonIndex=2;
      strcpy((char*)event->buttonText, map_[mygid].button2.c_str());

      gevent_EnqueueEvent(mygid, map_[mygid].callback, GUI_ALERT_DIALOG_COMPLETE_EVENT, 
			  event, 1, map_[mygid].udata);
      
      EndDialog(hwnd,LOWORD(wParam));
      return TRUE;
    }
    else if (LOWORD(wParam)==ID_BUTTON3){
      size_t size = sizeof(gui_AlertDialogCompleteEvent) + map_[mygid].cancelButton.length() + 1;
      event = (gui_AlertDialogCompleteEvent*)malloc(size);

      event->gid=mygid;
      event->buttonText=(char*)event+sizeof(gui_AlertDialogCompleteEvent);
      event->buttonIndex=0;
      strcpy((char*)event->buttonText, map_[mygid].cancelButton.c_str());

      gevent_EnqueueEvent(mygid, map_[mygid].callback, GUI_ALERT_DIALOG_COMPLETE_EVENT, 
			  event, 1, map_[mygid].udata);

      EndDialog(hwnd,LOWORD(wParam));
      return TRUE;
    }
    return FALSE;
  }
  else
    return FALSE;
}

extern "C" {

G_API void setWin32Stuff(HINSTANCE hInst, HWND hwnd)
{
  hInstcopy=hInst;
  hwndcopy=hwnd;
}

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

  Win32Dialog dialog;

  dialog.title=title;
  dialog.message=message;
  dialog.cancelButton=cancelButton;
  if (button1 != NULL) dialog.button1=button1;
  if (button2 != NULL) dialog.button2=button2;

  dialog.callback=callback;
  dialog.udata=udata;
  dialog.nbutton=1+(button1 != NULL)+(button2 != NULL);

  dialog.type=1;

  map_[gid]=dialog;

  printf("gui_createAlertDialog %d\n",gid);
  
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
  g_id gid = g_NextId();

  Win32Dialog dialog;

  dialog.title=title;
  dialog.message=message;
  dialog.cancelButton=cancelButton;
  if (button1 != NULL) dialog.button1=button1;
  if (button2 != NULL) dialog.button2=button2;

  dialog.callback=callback;
  dialog.udata=udata;
  dialog.nbutton=1+(button1 != NULL)+(button2 != NULL);

  dialog.text=text;
  dialog.type=2;

  map_[gid]=dialog;

  printf("gui_createTextInputDialog %d\n",gid);

  return gid;
}

G_API void gui_show(g_id gid)
{
  //  printf("gui_show, %d %d %p %p\n",gid,map_[gid].type,hInstcopy,hwndcopy);
  mygid = gid;

  int ret;
  if (map_[gid].type==1)
    ret=DialogBox(hInstcopy,MAKEINTRESOURCE(ID_ALERTDIALOG),hwndcopy,AlertDialogProc);
  else
    ret=DialogBox(hInstcopy,MAKEINTRESOURCE(ID_TEXTINPUTDIALOG),hwndcopy,TextInputDialogProc);

  if (ret==0 || ret==-1){
    DWORD dw=GetLastError();
    printf("return value DialogBox=%d, %d\n",ret,dw);
  }
}

G_API void gui_hide(g_id gid)
{
 //   s_manager->hide(gid);
}

G_API void gui_delete(g_id gid)
{
  std::map<g_id, Win32Dialog>::iterator iter = map_.find(gid);

  //  if (iter == map_.end())
  //    throw std::runtime_error("invalid gid");

  gevent_RemoveEventsWithGid(gid);

  map_.erase(iter);
}

G_API int gui_isVisible(g_id gid)
{
  return 0;
}

G_API void gui_setText(g_id gid, const char* text)
{
  if (map_[gid].type==2){
    map_[gid].text=text;
  }
}

G_API const char *gui_getText(g_id gid)
{
  if (map_[gid].type==2)
    return map_[gid].text.c_str();
  else
    return NULL;
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
