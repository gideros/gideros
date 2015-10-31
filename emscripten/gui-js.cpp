#include <gglobal.h>
#include <gevent.h>
#include <gui.h>
#include <emscripten.h>

#include <map>
#include <string>
#include <stdlib.h>


class UiDialog
{
public:
	std::string Title,Message;
	std::string Text;
	std::string ButtonC, Button1, Button2;
	bool input;
	bool IsVisible;
	bool IsSecure;
	void *udata;
	gevent_Callback callback;
};

static std::map<g_id, UiDialog> map_;


static void gui_eventAlert(g_id gid,int bi,const char *bt)
{
	size_t size = sizeof(gui_AlertDialogCompleteEvent) + strlen(bt) + 1;
	gui_AlertDialogCompleteEvent *event = (gui_AlertDialogCompleteEvent*)malloc(size);
	event->gid = gid;
	event->buttonText = (char*)event + sizeof(gui_AlertDialogCompleteEvent);
	event->buttonIndex=bi;
	strcpy((char*)event->buttonText, bt);

	gevent_EnqueueEvent(gid, map_[gid].callback, GUI_ALERT_DIALOG_COMPLETE_EVENT, event, 1, map_[gid].udata);
}

static void gui_eventInput(g_id gid,int bi,const char *bt,const char *t)
{
	size_t size = sizeof(gui_TextInputDialogCompleteEvent) + strlen(bt) + 1 + strlen(t)+1;
	gui_TextInputDialogCompleteEvent *event = (gui_TextInputDialogCompleteEvent*)malloc(size);
	event->gid = gid;
	event->buttonText = (char*)event + sizeof(gui_TextInputDialogCompleteEvent);
	event->text = (char*)event + sizeof(gui_TextInputDialogCompleteEvent)+strlen(bt)+1;
	event->buttonIndex=bi;
	strcpy((char*)event->buttonText, bt);
	strcpy((char*)event->text, t);

	gevent_EnqueueEvent(gid, map_[gid].callback, GUI_TEXT_INPUT_DIALOG_COMPLETE_EVENT, event, 1, map_[gid].udata);
}


extern "C" {

G_API void gui_init()
{
}

G_API void gui_cleanup()
{
 map_.clear();
}

G_API g_id gui_createAlertDialog(const char *title,
                                 const char *message,
                                 const char *cancelButton,
                                 const char *button1,
                                 const char *button2,
                                 gevent_Callback callback,
                                 void *udata)
{
	UiDialog d;
	d.Title=title;
	d.Message=message;
	d.ButtonC=cancelButton;
	d.Button1=button1?button1:"";
	d.Button2=button2?button2:"";
	d.callback=callback;
	d.udata=udata;
	d.input=false;
	d.IsVisible=false;
	d.IsSecure=false;

	g_id gid = g_NextId();
	map_[gid] = d;

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
	UiDialog d;
	d.Title=title;
	d.Message=message;
	d.ButtonC=cancelButton;
	d.Button1=button1?button1:"";
	d.Button2=button2?button2:"";
	d.callback=callback;
	d.udata=udata;
	d.Text=text?text:"";
	d.input=true;
	d.IsVisible=false;
	d.IsSecure=false;

	g_id gid = g_NextId();
	map_[gid] = d;

	return gid;
}

G_API void gui_show(g_id gid)
{
	std::map<g_id, UiDialog>::iterator iter = map_.find(gid);

	if (iter == map_.end())
		throw std::runtime_error("invalid gid");

	UiDialog *d=&(iter->second);
	if (d->IsVisible)
		return;
	d->IsVisible=true;
	
	if (!(d->input))
	{
/*		if (d->Button1.empty())
		{
			EM_ASM_({
				alert(Pointer_stringify($0));
				},d->Message.c_str());
			gui_eventAlert(gid,1,"OK");
		}
		else
		{
			int res=EM_ASM_INT({
				return confirm(Pointer_stringify($0));
				},d->Message.c_str());
			gui_eventAlert(gid,res?1:0,res?"OK":"Cancel");
		}*/
		EM_ASM_({
			var cb=$7;
			Module.gui_displayDialog($0,
			Pointer_stringify($1),
			Pointer_stringify($2),
			null,
			Pointer_stringify($3),
			$4?Pointer_stringify($4):null,
			$5?Pointer_stringify($5):null,
			$6,function(gid,bi,bt,t)
			{
			 var btj=allocate(intArrayFromString(bt), 'i8', ALLOC_STACK);
      			 Runtime.dynCall('viii', cb, [gid,bi,btj]);			      
			});
		},gid,d->Title.c_str(),d->Message.c_str(),
		d->ButtonC.c_str(),
		d->Button1.c_str(),
		d->Button2.empty()?0:d->Button2.c_str(),
		false,gui_eventAlert);
	}
	else
	{
	/*
		const char *res=(const char *) EM_ASM_INT({
			var t=prompt(Pointer_stringify($0),Pointer_stringify($1));
			if (t==null) return 0;
			return allocate(intArrayFromString(t), 'i8', ALLOC_STACK);
			},d->Message.c_str(),d->Text.c_str());
		if (res)
			d->Text=res;
		gui_eventInput(gid,res?1:0,res?"OK":"Cancel",d->Text.c_str());		*/
		EM_ASM_({
			var cb=$7;
			Module.gui_displayDialog($0,
			Pointer_stringify($1),
			Pointer_stringify($2),
			$8?Pointer_stringify($8):null,
			Pointer_stringify($3),
			$4?Pointer_stringify($4):null,
			$5?Pointer_stringify($5):null,
			$6,function(gid,bi,bt,t)
			{
			 var btj=allocate(intArrayFromString(bt), 'i8', ALLOC_STACK);
			 var tj=allocate(intArrayFromString(t), 'i8', ALLOC_STACK);
      			 Runtime.dynCall('viiii', cb, [gid,bi,btj,tj]);			      
			});
		},gid,d->Title.c_str(),d->Message.c_str(),
		d->ButtonC.c_str(),
		d->Button1.empty()?0:d->Button1.c_str(),
		d->Button2.empty()?0:d->Button2.c_str(),
		d->IsSecure,gui_eventInput,
		d->Text.empty()?0:d->Text.c_str());

	}
}

G_API void gui_hide(g_id gid)
{
	std::map<g_id, UiDialog>::iterator iter = map_.find(gid);

	if (iter == map_.end())
		throw std::runtime_error("invalid gid");
	if (iter->second.IsVisible)
	{
	 //Hide
		EM_ASM_({
			Module.gui_hideDialog($0);
		},gid);
	}
	iter->second.IsVisible=false;
}

G_API void gui_delete(g_id gid)
{
	std::map<g_id, UiDialog>::iterator iter = map_.find(gid);

	if (iter == map_.end())
		throw std::runtime_error("invalid gid");

	gui_hide(gid);
	gevent_RemoveEventsWithGid(gid);
	
	map_.erase(iter);
}

G_API int gui_isVisible(g_id gid)
{
	std::map<g_id, UiDialog>::iterator iter = map_.find(gid);
	if (iter == map_.end())
		throw std::runtime_error("invalid gid");
	return iter->second.IsVisible;
}

G_API void gui_setText(g_id gid, const char* text)
{
	std::map<g_id, UiDialog>::iterator iter = map_.find(gid);
	if (iter == map_.end())
		throw std::runtime_error("invalid gid");
	iter->second.Text=text;
}

G_API const char *gui_getText(g_id gid)
{
	std::map<g_id, UiDialog>::iterator iter = map_.find(gid);
	if (iter == map_.end())
		throw std::runtime_error("invalid gid");
	return iter->second.Text.c_str();
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
	std::map<g_id, UiDialog>::iterator iter = map_.find(gid);
	if (iter == map_.end())
		throw std::runtime_error("invalid gid");
	iter->second.IsSecure=secureInput;
}

G_API int gui_isSecureInput(g_id gid)
{
	std::map<g_id, UiDialog>::iterator iter = map_.find(gid);
	if (iter == map_.end())
		throw std::runtime_error("invalid gid");
	return iter->second.IsSecure?1:0;
}

}
