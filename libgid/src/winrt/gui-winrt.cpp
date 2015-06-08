#include <gglobal.h>
#include <gevent.h>
#include <gui.h>

#include "pch.h"

using namespace Windows::UI::Popups;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;


static gevent_Callback mycallback;
static void *myudata;
static g_id mygid;
static MessageDialog ^mymsg;

void CommandInvokedHandler(Windows::UI::Popups::IUICommand^ command)
{
//	size_t size = sizeof(gui_AlertDialogCompleteEvent) + command->Label->Length() + 1;
//	gui_AlertDialogCompleteEvent *event = (gui_AlertDialogCompleteEvent*)malloc(size);
//	event->gid = mygid;
//	event->buttonIndex = 1;
//	event->buttonText = (char*)event + sizeof(gui_AlertDialogCompleteEvent);

//	Platform::String ^string = command->Label;
//	const wchar_t *wstr=string->Data();

//	char str[10];
//	wcstombs(str, wstr, 10);

//	strcpy((char*)event->buttonText, str);

//	gevent_EnqueueEvent(mygid, mycallback, GUI_ALERT_DIALOG_COMPLETE_EVENT, event, 1, myudata);
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

	myudata = udata;
	mycallback = callback;
	mygid = g_NextId();

//	mymsg = ref new MessageDialog("message", "title");

	MessageDialog msg("message", "title");
	msg.ShowAsync();

//	UICommand^ cancelCommand = ref new UICommand(
//		"Cancel",
//		ref new UICommandInvokedHandler(&CommandInvokedHandler));

//	UICommand^ yesCommand = ref new UICommand(
//		"Yes",
//		ref new UICommandInvokedHandler(&CommandInvokedHandler));

//	UICommand^ noCommand = ref new UICommand(
//		"No",
//		ref new UICommandInvokedHandler(&CommandInvokedHandler));

	// Add the commands to the dialog
//	mymsg->Commands->Append(cancelCommand);
//	mymsg->Commands->Append(yesCommand);
//	mymsg->Commands->Append(noCommand);

	// Set the command that will be invoked by default
//	mymsg->DefaultCommandIndex = 0;

	// Set the command to be invoked when escape is pressed
//	mymsg->CancelCommandIndex = 1;

	return mygid;
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
	// Show the message dialog
//	mymsg->ShowAsync();
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
