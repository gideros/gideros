#include <jni.h>
#include "examplepluginbinder.h"

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class GExampleplugin {

public:
	GExampleplugin() {
		running=false;
		gid_ = g_NextId();
		JNIEnv *env = g_getJNIEnv();
		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/exampleplugin/GExampleplugin");
		cls_ = (jclass) env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);

		// i will need class and some methods from Java ArrayList
		jclass class_al = env->FindClass("java/util/ArrayList");
		ArrayList = (jclass)(env->NewGlobalRef(class_al));
		env->DeleteLocalRef(class_al);
		// no params, I for return integer
		ArrayList_size_id = env->GetMethodID (ArrayList, "size", "()I");
		//param integer, L fully-qualified-class-name for return class (=String)
		ArrayList_get_id = env->GetMethodID(ArrayList, "get", "(I)Ljava/lang/Object;");
	}
	~GExampleplugin() {
		stop();
		JNIEnv *env = g_getJNIEnv();
		env->DeleteGlobalRef(cls_);
	}
	void start() {
		JNIEnv *env = g_getJNIEnv();
		//no params, V for void
        env->CallStaticVoidMethod(cls_,env->GetStaticMethodID(cls_, "start_java", "()V"));
        running=true;
	}
	void stop() {
		if (running) {
			running=false;
			JNIEnv *env = g_getJNIEnv();
			//no params, V for void
			env->CallStaticVoidMethod(cls_,env->GetStaticMethodID(cls_, "stop_java", "()V"));
		}
	}
	bool test_plugin() {
		JNIEnv *env = g_getJNIEnv();
		//no params, Z for boolean
	    jboolean result = env->CallStaticBooleanMethod(cls_,env->GetStaticMethodID(cls_, "test_java", "()Z"));
	    return result;
	}
	jobject getPermissionsChecked() {
		JNIEnv *env = g_getJNIEnv();
		//no params, L fully-qualified-class-name for return class (=ArrayList)
		jobject result = (jobject) env->CallStaticObjectMethod(cls_,env->GetStaticMethodID(cls_, "getPermissionsChecked_java", "()Ljava/util/ArrayList;"));
		return result;
	}
	void onStateReceived(jint jstate, jstring jdescription){
		JNIEnv *env = g_getJNIEnv();
		/* EVENT gexampleplugin_state, defined in the .h file */
        // get C char from java string
		const char *d = env->GetStringUTFChars(jdescription, NULL);
        //set the C char in the struct
        gexampleplugin_state *event = (gexampleplugin_state*)gevent_CreateEventStruct1( //one string
			sizeof(gexampleplugin_state),//size of the struct
			offsetof(gexampleplugin_state, description), d);//setted in the field
		//release !
        env->ReleaseStringUTFChars(jdescription, d);
        //set jint in EVENT
        event->state=(int)jstate;
        /* EVENT in queue */
        gevent_EnqueueEvent(gid_, callback_s, GEXAMPLEPLUGIN_EVENT_STATE, event, 1, this);
    }
	void onWifiReceived(jstring jaction, jint jcount, jboolean jgranted, jobjectArray jpermissions, jobject jpermissions_checked){
		JNIEnv *env = g_getJNIEnv();

		/* jstring in the EVENT gexampleplugin_wifi, struct defined in the .h file */
		//get C char from java string
		const char *a = env->GetStringUTFChars(jaction, NULL);
		//set the C char in the EVENT
		size_t structSize = sizeof(gexampleplugin_wifi);
		gexampleplugin_wifi *event = (gexampleplugin_wifi*)gevent_CreateEventStruct1( //one string
			structSize,//size of the struct
			offsetof(gexampleplugin_wifi, action), a);//setted in the field "action"
		//release !
		env->ReleaseStringUTFChars(jaction, a);

		/* jint and jboolean  in the EVENT */
		event->count = (int)jcount;//setted in the field "count"
		event->granted = (bool)jgranted;//setted in the field "granted"

		/* jobjectArray in the EVENT */
		if (jpermissions != NULL) {
			//get C char from java string
			jstring j1 = (jstring) env->GetObjectArrayElement(jpermissions,0);//start at 0 (=java index)
			const char *c1 = env->GetStringUTFChars(j1, NULL);
			jstring j2 = (jstring) env->GetObjectArrayElement(jpermissions,1);
			const char *c2 = env->GetStringUTFChars(j2, NULL);
			jstring j3 = (jstring) env->GetObjectArrayElement(jpermissions,2);
			const char *c3 = env->GetStringUTFChars(j3, NULL);
			structSize = sizeof(gexampleplugin_wifi_permissions);
			//set C char in a struct, defined in .h file
			gexampleplugin_wifi_permissions *ps_all = (gexampleplugin_wifi_permissions*)gevent_CreateEventStruct3( //three strings (=max default)
				structSize,//size of the struct
				offsetof(gexampleplugin_wifi_permissions, change), c1,//setted in the field "change"
				offsetof(gexampleplugin_wifi_permissions, coarse), c2,//setted in the field "coarse"
				offsetof(gexampleplugin_wifi_permissions, fine), c3);//setted in the field "fine"
			//set the struct in the EVENT
			event->permissions_all = (gexampleplugin_wifi_permissions*)ps_all;//setted in the field "permissions_all"
			//release !
			env->ReleaseStringUTFChars(j1, c1);
			env->ReleaseStringUTFChars(j2, c2);
			env->ReleaseStringUTFChars(j3, c3);
			env->DeleteLocalRef(j1);
			env->DeleteLocalRef(j2);
			env->DeleteLocalRef(j3);
		}

		/* jobject (=ArrayList<String>) in the EVENT */
		if (jpermissions_checked != NULL) {
			//use method ArrayList .size(), defined in constructor GExampleplugin()
			jint len = env->CallIntMethod(jpermissions_checked, ArrayList_size_id);
			// set size of the list to can fetch from EVENT
			event->checked=(int)len;//setted in the field "permissions_checked_len"
			//alloc a struct, defined in .h file
			structSize = len * sizeof(gexampleplugin_wifi_permission);
			gexampleplugin_wifi_permission *ps_checked = (gexampleplugin_wifi_permission*)malloc(structSize);
			//set the struc in the EVENT
			event->permissions_checked = (gexampleplugin_wifi_permission*) ps_checked;//setted in the field "permissions_checked"
			for (jint i=0; i<len; i++) {//fetch the java list
				//use method ArrayList .get(index), defined in constructor GExampleplugin()
				jstring ji = (jstring) env->CallObjectMethod(jpermissions_checked, ArrayList_get_id, i);
				//get C char from java string
				const char* ci = env->GetStringUTFChars(ji, NULL);
				//set the C char in the same type of struct
				structSize = sizeof(gexampleplugin_wifi_permission);
				gexampleplugin_wifi_permission *pi = (gexampleplugin_wifi_permission*)gevent_CreateEventStruct1( //one string
					structSize,//size of the struct
					offsetof(gexampleplugin_wifi_permission, permission), ci);//setted in the field "permission"
				// release !
				env->ReleaseStringUTFChars(ji,ci);
				env->DeleteLocalRef(ji);
				//set the struct in the EVENT
				event->permissions_checked[i].permission=pi->permission;//set at index i
			}
		}

		//TODO jobject(=Map<key,Object>)

		/* EVENT in queue */
		gevent_EnqueueEvent(gid_, callback_s, GEXAMPLEPLUGIN_EVENT_WIFI, event, 1, this);
	}
private:
	static void callback_s(int type, void *event, void *udata)
	{
		((GExampleplugin*)udata)->callback(type, event);
	}
	void callback(int type, void *event) {
		gexampleplugin_dispatch(type, event);
	}
private:
	jclass cls_;
	g_id gid_;
	bool running;
	jclass ArrayList;
	jmethodID ArrayList_size_id;
	jmethodID ArrayList_get_id;
};

static GExampleplugin *_gexampleplugin = NULL;

extern "C" {
	JNIEXPORT void JNICALL Java_com_giderosmobile_android_plugins_exampleplugin_GExampleplugin_onState(
			JNIEnv *env, jclass clz,
			jint state, jstring description) {
		_gexampleplugin->onStateReceived(state,description);
	}
	JNIEXPORT void JNICALL Java_com_giderosmobile_android_plugins_exampleplugin_GExampleplugin_onWifi(
			JNIEnv *env, jclass clz,
			jstring action, jint count, jboolean granted, jobjectArray permissions_all) {
		//get ArrayList<String> from java
		jobject permissions_checked = _gexampleplugin->getPermissionsChecked();
		//Arraylist<String> added in params of event
		_gexampleplugin->onWifiReceived(action, count, granted, permissions_all, permissions_checked);
	}
}
void exampleplugin::init() {
	if (!_gexampleplugin)
		_gexampleplugin = new GExampleplugin;
}
void exampleplugin::deinit() {
	if (_gexampleplugin) {
		_gexampleplugin->stop();
		delete _gexampleplugin;
		_gexampleplugin = NULL;
	}
}
void exampleplugin::start(){
	_gexampleplugin->start();
}
void exampleplugin::stop() {
	if (_gexampleplugin)
		_gexampleplugin->stop();
}
bool exampleplugin::test_binder() {
	if (_gexampleplugin)
		return _gexampleplugin->test_plugin();
	return false;
}
