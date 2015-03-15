#include <notification_wrapper.h>
#include <jni.h>
#include <gapplication.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

static void *gevent_CreateEventStruct4(size_t structSize,
                                size_t offset1, const char *value1,
                                size_t offset2, const char *value2,
                                size_t offset3, const char *value3,
                                size_t offset4, const char *value4)
{
    size_t size1 = value1 ? (strlen(value1) + 1) : 0;
    size_t size2 = value2 ? (strlen(value2) + 1) : 0;
    size_t size3 = value3 ? (strlen(value3) + 1) : 0;
    size_t size4 = value4 ? (strlen(value4) + 1) : 0;

    void *result = malloc(structSize + size1 + size2 + size3 + size4);

    char **field1 = (char**)((char*)result + offset1);
    char **field2 = (char**)((char*)result + offset2);
    char **field3 = (char**)((char*)result + offset3);
    char **field4 = (char**)((char*)result + offset4);

    *field1 = value1 ? strcpy((char*)result + structSize,                 			value1) : NULL;
    *field2 = value2 ? strcpy((char*)result + structSize + size1,         			value2) : NULL;
    *field3 = value3 ? strcpy((char*)result + structSize + size1 + size2, 			value3) : NULL;
    *field4 = value3 ? strcpy((char*)result + structSize + size1 + size2 + size3, 	value4) : NULL;

    return result;
}

static const char* getString(JNIEnv *env, jstring jstr)
{
	const char *str = env->GetStringUTFChars(jstr, NULL);
	std::string result = str;
	env->ReleaseStringUTFChars(jstr, str);
	return result.c_str();
}

class GNotification
{
public:
	GNotification()
	{
		gid_ = g_NextId();
		
		JNIEnv *env = g_getJNIEnv();

		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/notification/NotificationClass");
		cls_ = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);
		
		jclass class_bundle = env->FindClass("android/os/Bundle");
		clsBundle = static_cast<jclass>(env->NewGlobalRef(class_bundle));
		env->DeleteLocalRef(class_bundle);
	
		jclass class_sparse = env->FindClass("android/util/SparseArray");
		clsSparse = static_cast<jclass>(env->NewGlobalRef(class_sparse));
		env->DeleteLocalRef(class_sparse);
		
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "construct", "(J)V"), (jlong)this);
		
		//subscribe to event
		gapplication_addCallback(onAppStart, this);
	}

	~GNotification()
	{
		JNIEnv *env = g_getJNIEnv();
		
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "destruct", "()V"));
		
		env->DeleteGlobalRef(cls_);
		env->DeleteGlobalRef(clsBundle);
		env->DeleteGlobalRef(clsSparse);
		
		gevent_RemoveEventsWithGid(gid_);
		gapplication_removeCallback(onAppStart, this);
	}
	
	void init(int id)
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "init", "(I)V"), (jint)id);
	}
	
	void cleanup(int id){
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cleanup", "(I)V"), (jint)id);
	}
	
	void set_title(int id, const char *title){
		JNIEnv *env = g_getJNIEnv();
		
		jstring jtitle= env->NewStringUTF(title);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setTitle", "(ILjava/lang/String;)V"), (jint)id, jtitle);
		env->DeleteLocalRef(jtitle);
	}
	
	const char* get_title(int id){
		JNIEnv *env = g_getJNIEnv();
		
		jstring js = (jstring)env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "getTitle", "(I)Ljava/lang/String;"), (jint)id);
		//const char *val = env->GetStringUTFChars(js, NULL);
		return getString(env, js);
	}
	
	void set_body(int id, const char *body){
		JNIEnv *env = g_getJNIEnv();
		
		jstring jbody = env->NewStringUTF(body);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setBody", "(ILjava/lang/String;)V"), (jint)id, jbody);
		env->DeleteLocalRef(jbody);
	}
	
	const char* get_body(int id){
		JNIEnv *env = g_getJNIEnv();
		
		jstring js = (jstring)env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "getBody", "(I)Ljava/lang/String;"), (jint)id);
		//const char *val = env->GetStringUTFChars(js, NULL);
		return getString(env, js);
	}
	
	void set_number(int id, int number){
		JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setNumber", "(II)V"), (jint)id, number);
	}
	
	int get_number(int id){
		JNIEnv *env = g_getJNIEnv();
		
		int val = (int)env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "getNumber", "(I)I"), (jint)id);
		return val;
	}
	
	void set_sound(int id, const char *sound){
		JNIEnv *env = g_getJNIEnv();
		
		jstring jsound = env->NewStringUTF(sound);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setSound", "(ILjava/lang/String;)V"), (jint)id, jsound);
		env->DeleteLocalRef(jsound);
	}
	
	const char* get_sound(int id){
		JNIEnv *env = g_getJNIEnv();
		
		jstring js = (jstring)env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "getSound", "(I)Ljava/lang/String;"), (jint)id);
		//const char *val = env->GetStringUTFChars(js, NULL);
		return getString(env, js);
	}
	
	void set_custom(int id, const char *custom){
		JNIEnv *env = g_getJNIEnv();
		
		jstring jcustom = env->NewStringUTF(custom);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setCustom", "(ILjava/lang/String;)V"), (jint)id, jcustom);
		env->DeleteLocalRef(jcustom);
	}
	
	const char* get_custom(int id){
		JNIEnv *env = g_getJNIEnv();
		
		jstring js = (jstring)env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "getCustom", "(I)Ljava/lang/String;"), (jint)id);
		//const char *val = env->GetStringUTFChars(js, NULL);
		return getString(env, js);
	}
	
	void dispatch_now(int id){
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "dispatchNow", "(I)V"), (jint)id);
	}
	
	void cancel(int id){
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cancel", "(I)V"), (jint)id);
	}
	
	void cancel_all(){
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cancelAll", "()V"));
	}
	
	void dispatch_after(int id, gnotification_Parameter *params1, gnotification_Parameter *params2){
		JNIEnv *env = g_getJNIEnv();
		
		//create Java Map object
		jobject jdispatchdate = env->NewObject(clsBundle, env->GetMethodID(clsBundle, "<init>", "()V"));
		while (params1->key)
		{
			jstring jKey = env->NewStringUTF(params1->key);
			jstring jVal = env->NewStringUTF(params1->value);
			env->CallVoidMethod(jdispatchdate, env->GetMethodID(clsBundle, "putString", "(Ljava/lang/String;Ljava/lang/String;)V"), jKey, jVal);
			env->DeleteLocalRef(jKey);
			env->DeleteLocalRef(jVal);
			++params1;
		}
		
		if(params2)
		{
			//create Java Map object
			jobject jrepeatdate = env->NewObject(clsBundle, env->GetMethodID(clsBundle, "<init>", "()V"));
			while (params2->key)
			{
				jstring jKey = env->NewStringUTF(params2->key);
				jstring jVal = env->NewStringUTF(params2->value);
				env->CallVoidMethod(jrepeatdate, env->GetMethodID(clsBundle, "putString", "(Ljava/lang/String;Ljava/lang/String;)V"), jKey, jVal);
				env->DeleteLocalRef(jKey);
				env->DeleteLocalRef(jVal);
				++params2;
			}
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "dispatchAfter", "(ILjava/lang/Object;Ljava/lang/Object;)V"), (jint)id, jdispatchdate, jrepeatdate);
			
			env->DeleteLocalRef(jrepeatdate);
		}
		else
		{
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "dispatchAfter", "(ILjava/lang/Object;)V"), (jint)id, jdispatchdate);
		}
		
		env->DeleteLocalRef(jdispatchdate);
	}
	
	void dispatch_on(int id, gnotification_Parameter *params1, gnotification_Parameter *params2){
		JNIEnv *env = g_getJNIEnv();
		
		//create Java Map object
		jobject jdispatchdate = env->NewObject(clsBundle, env->GetMethodID(clsBundle, "<init>", "()V"));
		while (params1->key)
		{
			jstring jKey = env->NewStringUTF(params1->key);
			jstring jVal = env->NewStringUTF(params1->value);
			env->CallVoidMethod(jdispatchdate, env->GetMethodID(clsBundle, "putString", "(Ljava/lang/String;Ljava/lang/String;)V"), jKey, jVal);
			env->DeleteLocalRef(jKey);
			env->DeleteLocalRef(jVal);
			++params1;
		}
		
		if(params2)
		{
			//create Java Map object
			jobject jrepeatdate = env->NewObject(clsBundle, env->GetMethodID(clsBundle, "<init>", "()V"));
			while (params2->key)
			{
				jstring jKey = env->NewStringUTF(params2->key);
				jstring jVal = env->NewStringUTF(params2->value);
				env->CallVoidMethod(jrepeatdate, env->GetMethodID(clsBundle, "putString", "(Ljava/lang/String;Ljava/lang/String;)V"), jKey, jVal);
				env->DeleteLocalRef(jKey);
				env->DeleteLocalRef(jVal);
				++params2;
			}
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "dispatchOn", "(ILjava/lang/Object;Ljava/lang/Object;)V"), (jint)id, jdispatchdate, jrepeatdate);
			
			env->DeleteLocalRef(jrepeatdate);
		}
		else
		{
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "dispatchOn", "(ILjava/lang/Object;)V"), (jint)id, jdispatchdate);
		}
		
		env->DeleteLocalRef(jdispatchdate);
	}
	
	void clear_local(){
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "clearLocalNotifications", "()V"));
	}
	
	void clear_push(){
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "clearPushNotifications", "()V"));
	}
	
	gnotification_Group* get_scheduled(){
		JNIEnv *env = g_getJNIEnv();
		jobject jmapobj = env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "getScheduledNotifications", "()Ljava/lang/Object;"));
		gnotification_Group *group = this->map2group(jmapobj);
		env->DeleteLocalRef(jmapobj);
		return group;
	}
	
	gnotification_Group* get_local(){
		JNIEnv *env = g_getJNIEnv();
		jobject jmapobj = env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "getLocalNotifications", "()Ljava/lang/Object;"));
		gnotification_Group *group = this->map2group(jmapobj);
		env->DeleteLocalRef(jmapobj);
		return group;
	}
	
	gnotification_Group* get_push(){
		JNIEnv *env = g_getJNIEnv();
		jobject jmapobj = env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "getPushNotifications", "()Ljava/lang/Object;"));
		gnotification_Group *group = this->map2group(jmapobj);
		env->DeleteLocalRef(jmapobj);
		return group;
	}
	
	void register_push(const char *project){
		JNIEnv *env = g_getJNIEnv();
		jstring jproject = env->NewStringUTF(project);
		env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "registerForPushNotifications", "(Ljava/lang/String;)V"), jproject);
		env->DeleteLocalRef(jproject);
	}
	
	void unregister_push(){
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "unregisterForPushNotifications", "()V"));
	}
	
	void ready_for_events(){
		JNIEnv *env = g_getJNIEnv();
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "readyForEvents", "()V"));
	}
	
	void onLocalNotification(jint id, jstring jtitle, jstring jtext, jint number, jstring jsound, jstring jcustom, jboolean didOpen)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *title = env->GetStringUTFChars(jtitle, NULL);
		const char *text = env->GetStringUTFChars(jtext, NULL);
		const char *sound = env->GetStringUTFChars(jsound, NULL);
		const char *custom = env->GetStringUTFChars(jcustom, NULL);
		
		gnotification_LocalEvent *event = (gnotification_LocalEvent*)gevent_CreateEventStruct4(
			sizeof(gnotification_LocalEvent),
			offsetof(gnotification_LocalEvent, title), title,
			offsetof(gnotification_LocalEvent, text), text,
			offsetof(gnotification_LocalEvent, sound), sound,
			offsetof(gnotification_LocalEvent, custom), custom);
			
		event->id = (int)id;
		event->number = (int)number;
		event->didOpen = (bool)didOpen;
	
		env->ReleaseStringUTFChars(jtitle, title);
		env->ReleaseStringUTFChars(jtext, text);
		env->ReleaseStringUTFChars(jsound, sound);
		env->ReleaseStringUTFChars(jcustom, custom);

		gevent_EnqueueEvent(gid_, callback_s, NOTIFICATION_LOCAL_EVENT, event, 1, this);
	}
	
	void onPushNotification(jint id, jstring jtitle, jstring jtext, jint number, jstring jsound, jstring jcustom, jboolean didOpen)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *title = env->GetStringUTFChars(jtitle, NULL);
		const char *text = env->GetStringUTFChars(jtext, NULL);
		const char *sound = env->GetStringUTFChars(jsound, NULL);
		const char *custom = env->GetStringUTFChars(jcustom, NULL);
		
		gnotification_PushEvent *event = (gnotification_PushEvent*)gevent_CreateEventStruct4(
			sizeof(gnotification_PushEvent),
			offsetof(gnotification_PushEvent, title), title,
			offsetof(gnotification_PushEvent, text), text,
			offsetof(gnotification_PushEvent, sound), sound,
			offsetof(gnotification_PushEvent, custom), custom);
			
		event->id = (int)id;
		event->number = (int)number;
		event->didOpen = (bool)didOpen;
	
		env->ReleaseStringUTFChars(jtitle, title);
		env->ReleaseStringUTFChars(jtext, text);
		env->ReleaseStringUTFChars(jsound, sound);
		env->ReleaseStringUTFChars(jcustom, custom);

		gevent_EnqueueEvent(gid_, callback_s, NOTIFICATION_PUSH_EVENT, event, 1, this);
	}
	
	void onPushRegistration(jstring jregId)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *regId = env->GetStringUTFChars(jregId, NULL);
		
		gnotification_RegisterPushEvent *event = (gnotification_RegisterPushEvent*)gevent_CreateEventStruct1(
			sizeof(gnotification_RegisterPushEvent),
			offsetof(gnotification_RegisterPushEvent, regId), regId);
	
		env->ReleaseStringUTFChars(jregId, regId);

		gevent_EnqueueEvent(gid_, callback_s, NOTIFICATION_PUSH_REGISTER_EVENT, event, 1, this);
	}
	
	void onPushRegistrationError(jstring jerrorId)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *errorId = env->GetStringUTFChars(jerrorId, NULL);
		
		gnotification_RegisterPushErrorEvent *event = (gnotification_RegisterPushErrorEvent*)gevent_CreateEventStruct1(
			sizeof(gnotification_RegisterPushErrorEvent),
			offsetof(gnotification_RegisterPushErrorEvent, errorId), errorId);
	
		env->ReleaseStringUTFChars(jerrorId, errorId);

		gevent_EnqueueEvent(gid_, callback_s, NOTIFICATION_PUSH_REGISTER_ERROR_EVENT, event, 1, this);
	}
	
	jstring transformPath(jstring jpath)
	{
		JNIEnv *env = g_getJNIEnv();
		
		const char *path = env->GetStringUTFChars(jpath, NULL);

		const char *gpath = g_pathForFile(path);
		
		env->ReleaseStringUTFChars(jpath, path);

		jstring jgpath = env->NewStringUTF(gpath);
			
		return jgpath;
	}
	
	
	//helping functions

	const char* mapGetStr(const char *str, jobject jsubobj)
	{
		JNIEnv *env = g_getJNIEnv();
		//get value
		jstring jStr = env->NewStringUTF(str);
		jstring jretStr = (jstring)env->CallObjectMethod(jsubobj, env->GetMethodID(clsBundle, "getString", "(Ljava/lang/String;)Ljava/lang/String;"), jStr);
		env->DeleteLocalRef(jStr);
	
		const char *retVal = env->GetStringUTFChars(jretStr, NULL);

		return retVal;
	}
	
	int mapGetInt(const char *str, jobject jsubobj)
	{
		JNIEnv *env = g_getJNIEnv();
		//get value
		jstring jStr = env->NewStringUTF(str);
		int ret = (int)env->CallIntMethod(jsubobj, env->GetMethodID(clsBundle, "getInt", "(Ljava/lang/String;)I"), jStr);
		env->DeleteLocalRef(jStr);
		
		return ret;
	}
	
	struct gnotification_Group* map2group(jobject jmapobj)
	{
		JNIEnv *env = g_getJNIEnv();
		int size = (int)env->CallIntMethod(jmapobj, env->GetMethodID(clsSparse, "size", "()I"));
		if(size == 0)
		{
			return NULL;
		}
		
		group.clear();
		
		for (int i = 0; i < size; i++) {
			int id = (int)env->CallIntMethod(jmapobj, env->GetMethodID(clsSparse, "keyAt", "(I)I"), (jint)i);
			jobject jsubobj = env->CallObjectMethod(jmapobj, env->GetMethodID(clsSparse, "valueAt", "(I)Ljava/lang/Object;"), (jint)i);
			
			gnotification_Group gparam = {id, this->mapGetStr("title", jsubobj), this->mapGetStr("message", jsubobj), this->mapGetInt("number", jsubobj), this->mapGetStr("sound", jsubobj), this->mapGetStr("custom", jsubobj)};
			
			group.push_back(gparam);
			
			env->DeleteLocalRef(jsubobj);
		}
		
		gnotification_Group param = {0, NULL, NULL, 0, NULL, NULL};
		group.push_back(param);
		
		return &group[0];
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
	static void callback_s(int type, void *event, void *udata)
	{
		((GNotification*)udata)->callback(type, event);
	}
	
	static void onAppStart(int type, void *event, void *udata)
	{
		if(type == GAPPLICATION_START_EVENT)
		{
			((GNotification*)udata)->ready_for_events();
		}
	}

	void callback(int type, void *event)
	{
		callbackList_.dispatchEvent(type, event);
	}

private:
	gevent_CallbackList callbackList_;

private:
	g_id gid_;
	jclass cls_;
	jclass clsBundle;
	jclass clsSparse;
	std::vector<gnotification_Group> group;
};

//C Wrapper

static GNotification *s_note = NULL;

extern "C" {

void gnotification_construct(){
	s_note = new GNotification;
}

void gnotification_destroy(){
	delete s_note;
	s_note = NULL;
}

void gnotification_init(int id){
	s_note->init(id);
}

void gnotification_cleanup(int id){
	if(s_note)
	{
		s_note->cleanup(id);
	}
}

void gnotification_set_title(int id, const char *title){
	s_note->set_title(id, title);
}

const char* gnotification_get_title(int id){
	return s_note->get_title(id);
}

void gnotification_set_body(int id, const char *body){
	s_note->set_body(id, body);
}

const char* gnotification_get_body(int id){
	return s_note->get_body(id);
}

void gnotification_set_number(int id, int number){
	s_note->set_number(id, number);
}

int gnotification_get_number(int id){
	return s_note->get_number(id);
}

void gnotification_set_sound(int id, const char *sound){
	s_note->set_sound(id, sound);
}

const char* gnotification_get_sound(int id){
	return s_note->get_sound(id);
}

void gnotification_set_custom(int id, const char *custom){
	s_note->set_custom(id, custom);
}

const char* gnotification_get_custom(int id){
	return s_note->get_custom(id);
}

void gnotification_dispatch_now(int id){
	s_note->dispatch_now(id);
}

void gnotification_cancel(int id){
	s_note->cancel(id);
}

void gnotification_cancel_all(){
	s_note->cancel_all();
}

void gnotification_dispatch_after(int id, gnotification_Parameter *params1, gnotification_Parameter *params2){
	s_note->dispatch_after(id, params1, params2);
}

void gnotification_dispatch_on(int id, gnotification_Parameter *params1, gnotification_Parameter *params2){
	s_note->dispatch_on(id, params1, params2);
}

void gnotification_clear_local(){
	s_note->clear_local();
}

void gnotification_clear_push(){
	s_note->clear_push();
}

gnotification_Group* gnotification_get_scheduled(){
	return s_note->get_scheduled();
}

gnotification_Group* gnotification_get_local(){
	return s_note->get_local();
}

gnotification_Group* gnotification_get_push(){
	return s_note->get_push();
}

void gnotification_register_push(const char *project){
	s_note->register_push(project);
}

void gnotification_unregister_push(){
	s_note->unregister_push();
}

g_id gnotification_addCallback(gevent_Callback callback, void *udata)
{
	return s_note->addCallback(callback, udata);
}

void gnotification_removeCallback(gevent_Callback callback, void *udata)
{
	s_note->removeCallback(callback, udata);
}

void gnotification_removeCallbackWithGid(g_id gid)
{
	s_note->removeCallbackWithGid(gid);
}

}

extern "C" {

	void Java_com_giderosmobile_android_plugins_notification_NotificationClass_onLocalNotification(JNIEnv *env, jclass clz, jint id, jstring jtitle, jstring jtext, jint number, jstring jsound, jstring jcustom, jboolean didOpen, jlong data)
	{
		((GNotification*)data)->onLocalNotification(id, jtitle, jtext, number, jsound, jcustom, didOpen);
	}
	
	void Java_com_giderosmobile_android_plugins_notification_NotificationClass_onPushNotification(JNIEnv *env, jclass clz, jint id, jstring jtitle, jstring jtext, jint number, jstring jsound, jstring jcustom, jboolean didOpen, jlong data)
	{
		((GNotification*)data)->onPushNotification(id, jtitle, jtext, number, jsound, jcustom, didOpen);
	}
	
	void Java_com_giderosmobile_android_plugins_notification_NotificationClass_onPushRegistration(JNIEnv *env, jclass clz, jstring jregId, jlong data)
	{
		((GNotification*)data)->onPushRegistration(jregId);
	}
	
	void Java_com_giderosmobile_android_plugins_notification_NotificationClass_onPushRegistrationError(JNIEnv *env, jclass clz, jstring jerrorId, jlong data)
	{
		((GNotification*)data)->onPushRegistrationError(jerrorId);
	}
	
	jstring Java_com_giderosmobile_android_plugins_notification_NotificationClass_transformPath(JNIEnv *env, jclass clz, jstring jpath, jlong data)
	{
		return ((GNotification*)data)->transformPath(jpath);
	}
}
