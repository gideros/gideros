#include <gglobal.h>
#include <gevent.h>
#include <gui.h>

#include "pch.h"
#include <map>

using namespace Windows::UI::Popups;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;

class AlertDialog
{
public:
	MessageDialog ^dialog;
	Platform::String ^Button1, ^Button2;
	void *udata;
	gevent_Callback callback;
};

static std::map<g_id, AlertDialog> map_;
static g_id mygid;

void CommandInvokedHandler(Windows::UI::Popups::IUICommand^ command)
{
	size_t size = sizeof(gui_AlertDialogCompleteEvent) + command->Label->Length() + 1;
	gui_AlertDialogCompleteEvent *event = (gui_AlertDialogCompleteEvent*)malloc(size);
	event->gid = mygid;
	event->buttonText = (char*)event + sizeof(gui_AlertDialogCompleteEvent);

	Platform::String ^string = command->Label;

	if (string == map_[mygid].Button1)
		event->buttonIndex = 1;
	else if (string == map_[mygid].Button2)
		event->buttonIndex = 2;
	else
		event->buttonIndex = 0;

	const wchar_t *wstr=string->Data();

	char *str;
	str = (char *)malloc((wcslen(wstr)+1)*sizeof(char));

	wcstombs(str, wstr, wcslen(wstr)+1);

	strcpy((char*)event->buttonText, str);

	free(str);

	gevent_EnqueueEvent(mygid, map_[mygid].callback, GUI_ALERT_DIALOG_COMPLETE_EVENT, event, 1, map_[mygid].udata);
}

extern "C" {

G_API void gui_init()
{
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
	Platform::String ^Title, ^Message, ^CancelButton, ^Button1, ^Button2;

	g_id gid = g_NextId();

	wchar_t *wmessage, *wtitle, *wcancelButton, *wbutton1, *wbutton2;
	wbutton1 = NULL;
	wbutton2 = NULL;

	wmessage = (wchar_t*)malloc((strlen(message) + 1)*sizeof(wchar_t));
	wtitle = (wchar_t*)malloc((strlen(title) + 1)*sizeof(wchar_t));
	wcancelButton = (wchar_t*)malloc((strlen(cancelButton) + 1)*sizeof(wchar_t));

	mbstowcs(wmessage, message, strlen(message)+1);
	mbstowcs(wtitle, title, strlen(title)+1);
	mbstowcs(wcancelButton, cancelButton, strlen(cancelButton)+1);

	Title = ref new Platform::String(wtitle);
	Message = ref new Platform::String(wmessage);
	CancelButton = ref new Platform::String(wcancelButton);

	MessageDialog ^msg = ref new MessageDialog(Message, Title);

	if (button1 != NULL) {
		wbutton1 = (wchar_t*)malloc((strlen(button1) + 1)*sizeof(wchar_t));
		mbstowcs(wbutton1, button1, strlen(button1)+1);
		Button1 = ref new Platform::String(wbutton1);

		UICommand^ button1Command = ref new UICommand(
			Button1,
			ref new UICommandInvokedHandler(&CommandInvokedHandler));

		msg->Commands->Append(button1Command);
	}

#if WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP
	if (button2 != NULL) {
		wbutton2 = (wchar_t*)malloc((strlen(button2) + 1)*sizeof(wchar_t));
		mbstowcs(wbutton2, button2, strlen(button2)+1);
		Button2 = ref new Platform::String(wbutton2);

		UICommand^ button2Command = ref new UICommand(
			Button2,
			ref new UICommandInvokedHandler(&CommandInvokedHandler));

		msg->Commands->Append(button2Command);
	}
#endif

	UICommand^ cancelCommand = ref new UICommand(
		CancelButton,
		ref new UICommandInvokedHandler(&CommandInvokedHandler));

	// Add the commands to the dialog
	msg->Commands->Append(cancelCommand);

	// Set the command that will be invoked by default
	msg->DefaultCommandIndex = 0;

	// Set the command to be invoked when escape is pressed
	if (button1==NULL && button2==NULL)
		msg->CancelCommandIndex = 0;
	else if (button2==NULL)
		msg->CancelCommandIndex = 1;
	else
		msg->CancelCommandIndex = 2;

	AlertDialog alertDialog;

	alertDialog.Button1 = Button1;
	alertDialog.Button2 = Button2;
	alertDialog.dialog = msg;
	alertDialog.callback = callback;
	alertDialog.udata = udata;

	map_[gid] = alertDialog;

	free(wmessage);
	free(wtitle);
	free(wcancelButton);
	if (wbutton1 != NULL) free(wbutton1);
	if (wbutton2 != NULL) free(wbutton2);

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
	mygid = gid;
}

G_API void gui_hide(g_id gid)
{
 //   s_manager->hide(gid);
}

G_API void gui_delete(g_id gid)
{
	std::map<g_id, AlertDialog>::iterator iter = map_.find(gid);

	if (iter == map_.end())
		throw std::runtime_error("invalid gid");

	gevent_RemoveEventsWithGid(gid);

	iter->second.dialog = nullptr;
	map_.erase(iter);

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
