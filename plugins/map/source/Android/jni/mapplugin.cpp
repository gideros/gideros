#include <mapplugin.h>
#include <jni.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class MapPluginCPPClass
{
	public:
		MapPluginCPPClass()
		{
			gid_ = g_NextId();
			
			JNIEnv *env = g_getJNIEnv();

			jclass localClass = env->FindClass("com/giderosmobile/android/plugins/mapplugin/MapPluginJava");
			cls_ = (jclass)env->NewGlobalRef(localClass);
			env->DeleteLocalRef(localClass);
			
			jclass class_bundle = env->FindClass("android/os/Bundle");
			clsBundle = static_cast<jclass>(env->NewGlobalRef(class_bundle));
			env->DeleteLocalRef(class_bundle);
			
			jclass class_sparse = env->FindClass("android/util/SparseArray");
			clsSparse = static_cast<jclass>(env->NewGlobalRef(class_sparse));
			env->DeleteLocalRef(class_sparse);

			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "init", "(J)V"), (jlong)this);
		}

		~MapPluginCPPClass()
		{
			JNIEnv *env = g_getJNIEnv();

			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cleanup", "()V"));
			
			env->DeleteGlobalRef(cls_);
			env->DeleteGlobalRef(clsBundle);
			env->DeleteGlobalRef(clsSparse);
			
			gevent_RemoveEventsWithGid(gid_);
		}
		
		void initialize()
		{
			JNIEnv *env = g_getJNIEnv();
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "initialize", "()V"));
		}
		
		void destroy(const char *mapplugin)
		{
			JNIEnv *env = g_getJNIEnv();
			jstring jiab = env->NewStringUTF(mapplugin);
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "destroy", "(Ljava/lang/String;)V"), jiab);
			env->DeleteLocalRef(jiab);
		}
		
		void mpcpp_setDimensions(const char *mapplugin, int width, int height)
		{
			JNIEnv *env = g_getJNIEnv();
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setDimensions", "(II)V"), width, height);
		}

		void mpcpp_setPosition(const char *mapplugin, int x, int y)
		{
			JNIEnv *env = g_getJNIEnv();
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setPosition", "(II)V"), x, y);
		}

		void mpcpp_hide(const char *mapplugin)
		{
			JNIEnv *env = g_getJNIEnv();
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "hide", "()V"));
		}

		void mpcpp_show(const char *mapplugin)
		{
			JNIEnv *env = g_getJNIEnv();
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "show", "()V"));
		}

		void mpcpp_clear(const char *mapplugin)
		{
			JNIEnv *env = g_getJNIEnv();
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "clear", "()V"));
		}

		void mpcpp_setCenterCoordinates(const char *mapplugin, double lat, double lon)
		{
			JNIEnv *env = g_getJNIEnv();
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setCenterCoordinates", "(DD)V"), lat, lon);
		}

		void mpcpp_setZoom(const char *mapplugin, int zoom)
		{
			JNIEnv *env = g_getJNIEnv();
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setZoom", "(I)V"), zoom);
		}

		int mpcpp_mapClicked(const char *mapplugin)
		{
			JNIEnv *env = g_getJNIEnv();
			return env->CallStaticIntMethod(cls_, env->GetStaticMethodID(cls_, "mapClicked", "()I"));
		}

		double mpcpp_getMapClickLatitude(const char *mapplugin)
		{
			JNIEnv *env = g_getJNIEnv();
			return env->CallStaticDoubleMethod(cls_, env->GetStaticMethodID(cls_, "getMapClickLatitude", "()D"));
		}

		double mpcpp_getMapClickLongitude(const char *mapplugin)
		{
			JNIEnv *env = g_getJNIEnv();
			return env->CallStaticDoubleMethod(cls_, env->GetStaticMethodID(cls_, "getMapClickLongitude", "()D"));
		}

		void mpcpp_setType(const char *mapplugin, int type)
		{
			JNIEnv *env = g_getJNIEnv();
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setType", "(I)V"), type);
		}

		double mpcpp_getCenterLatitude(const char *mapplugin)
		{
			JNIEnv *env = g_getJNIEnv();
			return env->CallStaticDoubleMethod(cls_, env->GetStaticMethodID(cls_, "getCenterLatitude", "()D"));
		}

		double mpcpp_getCenterLongitude(const char *mapplugin)
		{
			JNIEnv *env = g_getJNIEnv();
			return env->CallStaticDoubleMethod(cls_, env->GetStaticMethodID(cls_, "getCenterLongitude", "()D"));
		}

		int mpcpp_addMarker(const char *mapplugin, double lat, double lon, const char *title)
		{
			JNIEnv *env = g_getJNIEnv();
			return env->CallStaticIntMethod(cls_, env->GetStaticMethodID(cls_, "addMarker", "(DDLjava/lang/String;)I"), lat, lon, env->NewStringUTF(title));
		}

		void mpcpp_setMarkerTitle(const char *mapplugin, int idx, const char *title)
		{
			JNIEnv *env = g_getJNIEnv();
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setMarkerTitle", "(ILjava/lang/String;)V"), idx, env->NewStringUTF(title));
		}

		void mpcpp_setMarkerHue(const char *mapplugin, int idx, double hue)
		{
			JNIEnv *env = g_getJNIEnv();
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setMarkerHue", "(ID)V"), idx, hue);
		}

		void mpcpp_setMarkerAlpha(const char *mapplugin, int idx, double alpha)
		{
			JNIEnv *env = g_getJNIEnv();
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setMarkerAlpha", "(ID)V"), idx, alpha);
		}

		void mpcpp_setMarkerCoordinates(const char *mapplugin, int idx, double lat, double lon)
		{
			JNIEnv *env = g_getJNIEnv();
			env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setMarkerCoordinates", "(IDD)V"), idx, lat, lon);
		}

		const char *mpcpp_getMarkerTitle(const char *mapplugin, int idx)
		{
			JNIEnv *env = g_getJNIEnv();
			static jmethodID j_method = env->GetStaticMethodID(cls_, "getMarkerTitle", "(I)Ljava/lang/String;");

			return( (const char *)
					env->GetStringUTFChars
					(
						(jstring) env->CallStaticObjectMethod(cls_, j_method, idx), 0
					)
				);
		}

		double mpcpp_getMarkerLatitude(const char *mapplugin, int idx)
		{
			JNIEnv *env = g_getJNIEnv();
			return env->CallStaticDoubleMethod(cls_, env->GetStaticMethodID(cls_, "getMarkerLatitude", "(I)D"), idx);
		}

		double mpcpp_getMarkerLongitude(const char *mapplugin, int idx)
		{
			JNIEnv *env = g_getJNIEnv();
			return env->CallStaticDoubleMethod(cls_, env->GetStaticMethodID(cls_, "getMarkerLongitude", "(I)D"), idx);
		}

		int mpcpp_getClickedMarkerIndex(const char *mapplugin)
		{
			JNIEnv *env = g_getJNIEnv();
			return env->CallStaticIntMethod(cls_, env->GetStaticMethodID(cls_, "getClickedMarkerIndex", "()I"));
		}
		
		std::string MyGetStringUTFChars(JNIEnv *env, jstring jstr)
		{
			const char *str = env->GetStringUTFChars(jstr, NULL);
			std::string result = str;
			env->ReleaseStringUTFChars(jstr, str);
			return result;
		}
		

	private:
		jclass cls_;
		jclass clsBundle;
		jclass clsSparse;
		g_id gid_;
	};


	static MapPluginCPPClass *s_giab = NULL;

	extern "C" {

	void gmapplugin_init()
	{
		s_giab = new MapPluginCPPClass;
	}

	void gmapplugin_cleanup()
	{
		if(s_giab)
		{
			delete s_giab;
			s_giab = NULL;
		}
	}

	void gmapplugin_initialize()
	{
		if(s_giab)
		{
			s_giab->initialize();
		}
	}

	void gmapplugin_destroy(const char *mapplugin)
	{
		if(s_giab)
		{
			s_giab->destroy(mapplugin);
		}
	}

	void gmapplugin_setDimensions(const char *mapplugin, int width, int height)
	{
		if (s_giab)
		{
			s_giab->mpcpp_setDimensions(mapplugin, width, height);
		}
	}

	void gmapplugin_setPosition(const char *mapplugin, int x, int y){	if (s_giab) { s_giab->mpcpp_setPosition(mapplugin, x, y); } }
	void gmapplugin_hide(const char *mapplugin){	if (s_giab) { s_giab->mpcpp_hide(mapplugin); } }
	void gmapplugin_show(const char *mapplugin){	if (s_giab) { s_giab->mpcpp_show(mapplugin); } }
	void gmapplugin_clear(const char *mapplugin){	if (s_giab) { s_giab->mpcpp_clear(mapplugin); } }
	void gmapplugin_setCenterCoordinates(const char *mapplugin, double lat, double lon){	if (s_giab) { s_giab->mpcpp_setCenterCoordinates(mapplugin, lat, lon); } }
	void gmapplugin_setZoom(const char *mapplugin, int z){	if (s_giab) { s_giab->mpcpp_setZoom(mapplugin, z); } }
	int gmapplugin_mapClicked(const char *mapplugin){	if (s_giab) { return s_giab->mpcpp_mapClicked(mapplugin); } }
	double gmapplugin_getMapClickLatitude(const char *mapplugin){	if (s_giab) { return s_giab->mpcpp_getMapClickLatitude(mapplugin); } }
	double gmapplugin_getMapClickLongitude(const char *mapplugin){	if (s_giab) { s_giab->mpcpp_getMapClickLongitude(mapplugin); } }
	void gmapplugin_setType(const char *mapplugin, int t){	if (s_giab) { s_giab->mpcpp_setType(mapplugin, t); } }
	double gmapplugin_getCenterLatitude(const char *mapplugin){	if (s_giab) { return s_giab->mpcpp_getCenterLatitude(mapplugin); } }
	double gmapplugin_getCenterLongitude(const char *mapplugin){	if (s_giab) { return s_giab->mpcpp_getCenterLongitude(mapplugin); } }
	int gmapplugin_addMarker(const char *mapplugin, double lat, double lon, const char *title){	if (s_giab) { return s_giab->mpcpp_addMarker(mapplugin, lat, lon, title); } }
	void gmapplugin_setMarkerTitle(const char *mapplugin, int idx, const char *title){	if (s_giab) { s_giab->mpcpp_setMarkerTitle(mapplugin, idx, title); } }
	void gmapplugin_setMarkerHue(const char *mapplugin, int idx, double hue){	if (s_giab) { s_giab->mpcpp_setMarkerHue(mapplugin, idx, hue); } }
	void gmapplugin_setMarkerAlpha(const char *mapplugin, int idx, double alpha){	if (s_giab) { s_giab->mpcpp_setMarkerAlpha(mapplugin, idx, alpha); } }
	void gmapplugin_setMarkerCoordinates(const char *mapplugin, int idx, double lat, double lon){	if (s_giab) { s_giab->mpcpp_setMarkerCoordinates(mapplugin, idx, lat, lon); } }
	const char *gmapplugin_getMarkerTitle(const char *mapplugin, int idx){	if (s_giab) { return s_giab->mpcpp_getMarkerTitle(mapplugin, idx); } }
	double gmapplugin_getMarkerLatitude(const char *mapplugin, int idx){	if (s_giab) { return s_giab->mpcpp_getMarkerLatitude(mapplugin, idx); } }
	double gmapplugin_getMarkerLongitude(const char *mapplugin, int idx){	if (s_giab) { return s_giab->mpcpp_getMarkerLongitude(mapplugin, idx); } }
	int gmapplugin_getClickedMarkerIndex(const char *mapplugin){	if (s_giab) { return s_giab->mpcpp_getClickedMarkerIndex(mapplugin); } }

}
