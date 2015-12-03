#include "gfacebook.h"
#include <stdlib.h>
#include <glog.h>
#include <gstdio.h>
#include <string>
#include <gapplication.h>
#include <emscripten.h>
#include <emscripten/val.h>

using namespace emscripten;

/*
static const char * const Base64keyStr =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

static char *Base64Encode(const void *data, int size) {
 int bsz = ((size + 2) / 3) * 4;
 char *b = (char *) umalloc(bsz + 1, "Base64");
 if (!b)
  return NULL;
 b[Base64EncodeToBuffer(data, size, b, bsz)] = 0;
 return b;
}

static int Base64EncodeToBuffer(const void *data, int size, char *buffer, int maxsize) {
 int i = 0, osz = 0;
 char enc[4];
 unsigned char *bd=(unsigned char *)data;
 while (i < size) {
  unsigned int m = bd[i++];
  enc[0] = Base64keyStr[((m >> 2) & 0x3f)];
  if (i < size) {
   m = ((m & 0x03) << 8) | (bd[i++] & 0xFF);
   enc[1] = Base64keyStr[(m >> 4) & 0x3f];
   if (i < size) {
    m = ((m & 0x0F) << 8) | (bd[i++] & 0xFF);
    enc[2] = Base64keyStr[(m >> 6) & 0x3f];
    enc[3] = Base64keyStr[m & 0x3f];
   } else {
    enc[2] = Base64keyStr[(m << 2) & 0x3f];
    enc[3] = '=';
   }
  } else {
   enc[1] = Base64keyStr[(m << 4) & 0x3f];
   enc[2] = '=';
   enc[3] = '=';
  }
  int cn = maxsize;
  if (cn > 4)
   cn = 4;
  memcpy(buffer, enc, cn);
  buffer += cn;
  maxsize -= cn;
  osz += cn;
 }
 return osz;
}*/                                            


static std::string FileToString(const char * file)
{
 G_FILE *f=g_fopen(file,"rb");
// glog_i("Open file:%s -> %p",file,f);
 if (!f)
  return std::string("");
 g_fseek(f,0,SEEK_END);
 long l=g_ftell(f);
 g_fseek(f,0,SEEK_SET);
 char *s=(char *) malloc(l);
 l=g_fread(s,1,l,f);
 g_fclose(f);
// glog_i("File size:%d",l);
 std::string ss(s,l);
 free(s);
 return ss;
}

class GGFacebook
{
public:
    GGFacebook()
    {
		gid_ = g_NextId();
		EM_ASM(GiderosFB.Init(););
        gapplication_addCallback(openUrl_s, this);
    }
    
    ~GGFacebook()
    {
        gapplication_removeCallback(openUrl_s, this);
		EM_ASM(GiderosFB.Deinit(););
		gevent_RemoveEventsWithGid(gid_);
    }
	
    void login(const char *appId, const char * const *permissions)
    {
	    val gfb = val::global("GiderosFB");
	    val perms=val::array();
	    int n=0;
	    if (permissions)
	     while (*permissions)
	     {
	      perms.set(n++,val(*permissions));
	      permissions++;
	     }
	    gfb.call<void>("Login",val(appId),perms);
    }
    
    void logout()
    {
		EM_ASM(GiderosFB.Logout(););
    }
    
    void upload(const char *path, const char *orig)
    {
	    val gfb = val::global("GiderosFB");
	    gfb.call<void>("Upload",val(path),val(orig));
    }
    
    const char* getAccessToken(){
	    val gfb = val::global("GiderosFB");
        return gfb.call<std::string>("GetAccessToken").c_str();
    }
    
    time_t getExpirationDate(){
	    val gfb = val::global("GiderosFB");
        return gfb.call<int>("GetExpirationDate");
    }
    
    void dialog(const char *action, const gfacebook_Parameter *params)
    {
	    val gfb = val::global("GiderosFB");
	    val p=val::object();
	    if (params)
	     while (*params->key)
	     {
	      p.set(val(params->key),val(params->value));
	      params++;
	     }
	    gfb.call<void>("Dialog",val(action),p);
    }

    void request(const char *graphPath, const gfacebook_Parameter *params, int httpMethod)
    {
	    val gfb = val::global("GiderosFB");
	    val p=val::object();
	    const char *mtd="get";
	    switch (httpMethod)
	    {
	     case 1: mtd="post"; break;
	     case 2: mtd="delete"; break;
	    }
	    
	    if (params)
	     while (*params->key)
	     {
	      if (!strcmp(params->key,"path"))
	      {
	       val a=val::object();
	       const char *ext=strchr(params->value,'.');
	       a.set(val("name"),val("source"));
	       a.set(val("type"),val(ext));
	       a.set(val("data"),FileToString(params->value));
	       p.set(val("source"),a);
              }
	      else
	       p.set(val(params->key),val(params->value));
	      params++;
	     }
	    gfb.call<void>("Request",val(graphPath),val(mtd),p);
    }
	
    void onLoginComplete()
    {
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGIN_COMPLETE_EVENT, NULL, 0, this);
    }
	
	void onLoginError(const char* value)
    {
		
        gfacebook_SimpleEvent *event = (gfacebook_SimpleEvent*)gevent_CreateEventStruct1(
            sizeof(gfacebook_SimpleEvent),
            offsetof(gfacebook_SimpleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGIN_ERROR_EVENT, event, 1, this);
    }
	
	void onLogoutComplete()
    {
       gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGOUT_COMPLETE_EVENT, NULL, 0, this);
    }
	
	void onLogoutError(const char* value)
    {
        gfacebook_SimpleEvent *event = (gfacebook_SimpleEvent*)gevent_CreateEventStruct1(
            sizeof(gfacebook_SimpleEvent),
            offsetof(gfacebook_SimpleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_LOGOUT_ERROR_EVENT, event, 1, this);
    }
	
	void onDialogComplete(const char* type, const char* value)
    {
        gfacebook_DoubleEvent *event = (gfacebook_DoubleEvent*)gevent_CreateEventStruct2(sizeof(gfacebook_DoubleEvent),
            offsetof(gfacebook_DoubleEvent, type), type,
            offsetof(gfacebook_DoubleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_DIALOG_COMPLETE_EVENT, event, 1, this);
    }
	
	void onDialogError(const char* type, const char* value)
    {
        gfacebook_DoubleEvent *event = (gfacebook_DoubleEvent*)gevent_CreateEventStruct2(
			sizeof(gfacebook_DoubleEvent),
			offsetof(gfacebook_DoubleEvent, type), type,
			offsetof(gfacebook_DoubleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_DIALOG_ERROR_EVENT, event, 1, this);
    }
	
	void onRequestError(const char* type, const char* value)
    {
        gfacebook_DoubleEvent *event = (gfacebook_DoubleEvent*)gevent_CreateEventStruct2(
			sizeof(gfacebook_DoubleEvent),
			offsetof(gfacebook_DoubleEvent, type), type,
			offsetof(gfacebook_DoubleEvent, value), value);
        
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_REQUEST_ERROR_EVENT, event, 1, this);

    }
	
	void onRequestComplete(const char* type, const char* response)
    {
		gfacebook_ResponseEvent *event = (gfacebook_ResponseEvent*)gevent_CreateEventStruct2(
			sizeof(gfacebook_ResponseEvent),
			offsetof(gfacebook_ResponseEvent, type), type,
			offsetof(gfacebook_ResponseEvent, response), response);
        
		event->responseLength = strlen(response);
		
        gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_REQUEST_COMPLETE_EVENT, event, 1, this);
    }   
    
	g_id addCallback(gevent_Callback callback, void *udata)
	{
		return callbackList_.addCallback(callback, udata);
	}
	void removeCallback(gevent_Callback callback, void *udata)
	{
		callbackList_.removeCallback(callback, udata);
	}
	void removeCallbackWithGid(g_id gid)
	{
		callbackList_.removeCallbackWithGid(gid);
	}
    
private:
    static void openUrl_s(int type, void *event, void *udata)
    {
        static_cast<GGFacebook*>(udata)->openUrl(type, event);
    }
    
    void openUrl(int type, void *event)
    {
        if (type == GAPPLICATION_OPEN_URL_EVENT)
        {
                gapplication_OpenUrlEvent *event2 = (gapplication_OpenUrlEvent*)event;
                
                const char* url = event2->url;
                
                //[fb handleOpenUrl:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
                
                gfacebook_SimpleEvent *event3 = (gfacebook_SimpleEvent*)gevent_CreateEventStruct1(
                    sizeof(gfacebook_SimpleEvent),
                    offsetof(gfacebook_SimpleEvent, value), url);
                
                gevent_EnqueueEvent(gid_, callback_s, GFACEBOOK_OPEN_URL_EVENT, event3, 1, this);
        }
        else if(type == GAPPLICATION_RESUME_EVENT){
/*            if(fb != NULL)
            {
                [fb applicationDidBecomeActive];
            }
            */
        }
    }
	static void callback_s(int type, void *event, void *udata)
	{
		((GGFacebook*)udata)->callback(type, event);
	}

	void callback(int type, void *event)
	{
		callbackList_.dispatchEvent(type, event);
	}

private:
	gevent_CallbackList callbackList_;

private:
	g_id gid_;
	std::string accessToken_;
};

static GGFacebook *s_facebook = NULL;

extern "C" {

void gfacebook_init()
{
    s_facebook = new GGFacebook;
}

void gfacebook_cleanup()
{
    delete s_facebook;
    s_facebook = NULL;
}

void gfacebook_login(const char *appId, const char * const *permissions)
{
    s_facebook->login(appId, permissions);
}

void gfacebook_logout()
{
    s_facebook->logout();
}
    
const char* gfacebook_getAccessToken(){
    return s_facebook->getAccessToken();
}
    
time_t gfacebook_getExpirationDate(){
    return s_facebook->getExpirationDate();
}
    
void gfacebook_dialog(const char *action, const gfacebook_Parameter *params)
{
    s_facebook->dialog(action, params);
}

void gfacebook_request(const char *graphPath, const gfacebook_Parameter *params, int httpMethod)
{
    s_facebook->request(graphPath, params, httpMethod);
}
    
void gfacebook_upload(const char *path, const char *orig)
{
   s_facebook->upload(path, orig);
}
    
void gfacebook_onLoginComplete(){
    s_facebook->onLoginComplete();
}
void gfacebook_onLoginError(const char* value){
    s_facebook->onLoginError(value);
}
void gfacebook_onLogoutComplete(){
    s_facebook->onLogoutComplete();
}
void gfacebook_onLogoutError(const char* value){
    s_facebook->onLogoutError(value);
}
void gfacebook_onDialogComplete(const char* type, const char* value){
    s_facebook->onDialogComplete(type,value);
}
void gfacebook_onDialogError(const char* type, const char* value){
    s_facebook->onDialogError(type, value);
}
void gfacebook_onRequestError(const char* type, const char* value){
    s_facebook->onRequestError(type, value);
}
void gfacebook_onRequestComplete(const char* type, const char* response){
    s_facebook->onRequestComplete(type, response);
}

g_id gfacebook_addCallback(gevent_Callback callback, void *udata)
{
	return s_facebook->addCallback(callback, udata);
}

void gfacebook_removeCallback(gevent_Callback callback, void *udata)
{
	s_facebook->removeCallback(callback, udata);
}

void gfacebook_removeCallbackWithGid(g_id gid)
{
	s_facebook->removeCallbackWithGid(gid);
}

}
