#include <iab.h>
#include <jni.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class GIab
{
public:
	GIab()
	{
		gid_ = g_NextId();
		
		JNIEnv *env = g_getJNIEnv();

		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/iab/Iab");
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

	~GIab()
	{
		JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cleanup", "()V"));
		
		env->DeleteGlobalRef(cls_);
		env->DeleteGlobalRef(clsBundle);
		env->DeleteGlobalRef(clsSparse);
		
		gevent_RemoveEventsWithGid(gid_);
	}
	
	giab_SimpleParam* detectStores(giab_SimpleParam *params)
	{
		JNIEnv *env = g_getJNIEnv();
		jobject jparams = env->NewObject(clsSparse, env->GetMethodID(clsSparse, "<init>", "()V"));
		int i = 0;
		while (!params->value.empty())
		{
			jstring jVal = env->NewStringUTF(params->value.c_str());
			env->CallVoidMethod(jparams, env->GetMethodID(clsSparse, "put", "(ILjava/lang/Object;)V"), (jint)i, jVal);
			env->DeleteLocalRef(jVal);
			++params;
			i++;
		}
		
		jobject jstores = env->CallStaticObjectMethod(cls_, env->GetStaticMethodID(cls_, "detectStores", "(Ljava/lang/Object;)Ljava/lang/Object;"), jparams);
		int size = (int)env->CallIntMethod(jstores, env->GetMethodID(clsSparse, "size", "()I"));
		if(size == 0)
		{
			return NULL;
		}
		
		stores.clear();
		
		for (int i = 0; i < size; i++) {
			jstring jstore = (jstring)env->CallObjectMethod(jstores, env->GetMethodID(clsSparse, "valueAt", "(I)Ljava/lang/Object;"), (jint)i);
			const char *store = env->GetStringUTFChars(jstore, NULL);
			giab_SimpleParam param = {store};
			env->ReleaseStringUTFChars(jstore, store);
			stores.push_back(param);
			env->DeleteLocalRef(jstore);
		}
		giab_SimpleParam param = {""};
		stores.push_back(param);
		
		env->DeleteLocalRef(jstores);
		
		return &stores[0];
	}
	
	void initialize(const char *iab)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jiab = env->NewStringUTF(iab);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "initialize", "(Ljava/lang/String;)V"), jiab);
		env->DeleteLocalRef(jiab);
	}
	
	void destroy(const char *iab)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jiab = env->NewStringUTF(iab);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "destroy", "(Ljava/lang/String;)V"), jiab);
		env->DeleteLocalRef(jiab);
	}
	
	void setup(const char *iab, giab_Parameter *params)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jiab = env->NewStringUTF(iab);
		//create Java object
		jobject jparams = env->NewObject(clsSparse, env->GetMethodID(clsSparse, "<init>", "()V"));
		int i = 0;
		while (params->value)
		{
			//jstring jVal = env->NewStringUTF(params->value);
			jbyteArray jVal = env->NewByteArray(params->size);
			env->SetByteArrayRegion(jVal, 0, params->size, (const jbyte*)params->value);
			env->CallVoidMethod(jparams, env->GetMethodID(clsSparse, "put", "(ILjava/lang/Object;)V"), (jint)i, jVal);
			env->DeleteLocalRef(jVal);
			++params;
			i++;
		}
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setup", "(Ljava/lang/String;Ljava/lang/Object;)V"), jiab, jparams);
		env->DeleteLocalRef(jparams);
		env->DeleteLocalRef(jiab);
	}
	
	void check(const char *iab)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jiab = env->NewStringUTF(iab);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "check", "(Ljava/lang/String;)V"), jiab);
		env->DeleteLocalRef(jiab);		
	}
	
	void setProducts(const char *iab, giab_DoubleEvent *params){
		JNIEnv *env = g_getJNIEnv();
		jstring jiab = env->NewStringUTF(iab);
		jobject jparams = env->NewObject(clsSparse, env->GetMethodID(clsSparse, "<init>", "()V"));
		jobject jparams2 = env->NewObject(clsSparse, env->GetMethodID(clsSparse, "<init>", "()V"));
		int i = 0;
		while (params->iap)
		{
			jstring jKey = env->NewStringUTF(params->iap);
			jstring jVal = env->NewStringUTF(params->value);
			env->CallVoidMethod(jparams, env->GetMethodID(clsSparse, "put", "(ILjava/lang/Object;)V"), (jint)i, jKey);
			env->CallVoidMethod(jparams2, env->GetMethodID(clsSparse, "put", "(ILjava/lang/Object;)V"), (jint)i, jVal);
			env->DeleteLocalRef(jKey);
			env->DeleteLocalRef(jVal);
			params++;
			i++;
		}
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setProducts", "(Ljava/lang/String;Ljava/lang/Object;Ljava/lang/Object;)V"), jiab, jparams, jparams2);
		env->DeleteLocalRef(jparams);
		env->DeleteLocalRef(jparams2);
		env->DeleteLocalRef(jiab);
	}
	
	void setConsumables(const char *iab, const char * const *params){
		JNIEnv *env = g_getJNIEnv();
		jstring jiab = env->NewStringUTF(iab);
		jobject jparams = env->NewObject(clsSparse, env->GetMethodID(clsSparse, "<init>", "()V"));
		int i = 0;
		while (*params)
		{
			jstring jVal = env->NewStringUTF(*params);
			env->CallVoidMethod(jparams, env->GetMethodID(clsSparse, "put", "(ILjava/lang/Object;)V"), (jint)i, jVal);
			env->DeleteLocalRef(jVal);
			params++;
			i++;
		}
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "setConsumables", "(Ljava/lang/String;Ljava/lang/Object;)V"), jiab, jparams);
		env->DeleteLocalRef(jparams);
		env->DeleteLocalRef(jiab);
	}
	
	void request(const char *iab){
		JNIEnv *env = g_getJNIEnv();
		jstring jiab = env->NewStringUTF(iab);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "request", "(Ljava/lang/String;)V"), jiab);
		env->DeleteLocalRef(jiab);
	}
	
	void purchase(const char *iab, const char *productId)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jiab = env->NewStringUTF(iab);
		jstring jproductId = env->NewStringUTF(productId);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "purchase", "(Ljava/lang/String;Ljava/lang/String;)V"), jiab, jproductId);
		env->DeleteLocalRef(jproductId);
		env->DeleteLocalRef(jiab);
	}
	
	void restore(const char *iab)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jiab = env->NewStringUTF(iab);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "restore", "(Ljava/lang/String;)V"), jiab);	
		env->DeleteLocalRef(jiab);
	}
	
	std::string mapGetStr(const char *str, jobject jsubobj)
	{
		JNIEnv *env = g_getJNIEnv();
		//get value
		jstring jStr = env->NewStringUTF(str);
		jstring jretStr = (jstring)env->CallObjectMethod(jsubobj, env->GetMethodID(clsBundle, "getString", "(Ljava/lang/String;)Ljava/lang/String;"), jStr);
		env->DeleteLocalRef(jStr);
	
		const char *retVal = env->GetStringUTFChars(jretStr, NULL);
		std::string result = retVal;
		env->ReleaseStringUTFChars(jretStr, retVal);

		return result;
	}
	
	void map2products(jobject jmapobj)
	{
		JNIEnv *env = g_getJNIEnv();
		int size = (int)env->CallIntMethod(jmapobj, env->GetMethodID(clsSparse, "size", "()I"));
		if(size == 0)
		{
			return;
		}
		
		products.clear();
		
		for (int i = 0; i < size; i++) {
			jobject jsubobj = env->CallObjectMethod(jmapobj, env->GetMethodID(clsSparse, "valueAt", "(I)Ljava/lang/Object;"), (jint)i);
			
			Product gprod = {this->mapGetStr("productId", jsubobj), this->mapGetStr("title", jsubobj), this->mapGetStr("description", jsubobj), this->mapGetStr("price", jsubobj)};
			
			products.push_back(gprod);
			
			env->DeleteLocalRef(jsubobj);
		}
	}
	
	std::string MyGetStringUTFChars(JNIEnv *env, jstring jstr)
	{
		const char *str = env->GetStringUTFChars(jstr, NULL);
		std::string result = str;
		env->ReleaseStringUTFChars(jstr, str);
		return result;
	}
	
	void onAvailable(jstring jIap)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *iap = env->GetStringUTFChars(jIap, NULL);
		giab_SimpleEvent *event = (giab_SimpleEvent*)gevent_CreateEventStruct1(
			sizeof(giab_SimpleEvent),
			offsetof(giab_SimpleEvent, iap), iap);
		env->ReleaseStringUTFChars(jIap, iap);
		gevent_EnqueueEvent(gid_, callback_s, GIAB_AVAILABLE_EVENT, event, 1, this);
	}
	
	void onNotAvailable(jstring jIap)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *iap = env->GetStringUTFChars(jIap, NULL);
		giab_SimpleEvent *event = (giab_SimpleEvent*)gevent_CreateEventStruct1(
			sizeof(giab_SimpleEvent),
			offsetof(giab_SimpleEvent, iap), iap);
		env->ReleaseStringUTFChars(jIap, iap);
		gevent_EnqueueEvent(gid_, callback_s, GIAB_NOT_AVAILABLE_EVENT, event, 1, this);
	}
	
	void onPurchaseComplete(jstring jIap, jstring jproduct, jstring jreceipt)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *iap = env->GetStringUTFChars(jIap, NULL);
		const char *productId = env->GetStringUTFChars(jproduct, NULL);
		const char *receiptId = env->GetStringUTFChars(jreceipt, NULL);
		giab_Purchase *event = (giab_Purchase*)gevent_CreateEventStruct3(
			sizeof(giab_Purchase),
			offsetof(giab_Purchase, iap), iap,
			offsetof(giab_Purchase, productId), productId,
			offsetof(giab_Purchase, receiptId), receiptId);
		
		env->ReleaseStringUTFChars(jIap, iap);
		env->ReleaseStringUTFChars(jproduct, productId);
		env->ReleaseStringUTFChars(jreceipt, receiptId);
		gevent_EnqueueEvent(gid_, callback_s, GIAB_PURCHASE_COMPLETE_EVENT, event, 1, this);
	}
	
	void onPurchaseError(jstring jIap, jstring jerror)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *iap = env->GetStringUTFChars(jIap, NULL);
		const char *value = env->GetStringUTFChars(jerror, NULL);
		giab_DoubleEvent *event = (giab_DoubleEvent*)gevent_CreateEventStruct2(
			sizeof(giab_DoubleEvent),
			offsetof(giab_DoubleEvent, iap), iap,
			offsetof(giab_DoubleEvent, value), value);
		env->ReleaseStringUTFChars(jIap, iap);
		env->ReleaseStringUTFChars(jerror, value);
		gevent_EnqueueEvent(gid_, callback_s, GIAB_PURCHASE_ERROR_EVENT, event, 1, this);
	}
	
	void onProductsComplete(jstring jIap, jobject jproducts)
	{
		JNIEnv *env = g_getJNIEnv();
		this->map2products(jproducts);
		
		size_t size = sizeof(giab_Products);
		int count = (int)products.size();
		
		for (std::size_t i = 0; i < count; ++i)
		{
			size += sizeof(giab_Product);
			size += products[i].productId.size() + 1;
			size += products[i].title.size() + 1;
			size += products[i].description.size() + 1;
			size += products[i].price.size() + 1;
		}
		
		std::string iap = MyGetStringUTFChars(env, jIap);	
		size += iap.size() + 1;
		
		// allocate it
		giab_Products *event = (giab_Products*)malloc(size);
		
		// and copy the data into it
		char *ptr = (char*)event + sizeof(giab_Products);
		
		event->iap = ptr;
		strcpy(ptr, iap.c_str());
		ptr += iap.size() + 1;
		
		event->count = count;
		event->products = (giab_Product*)ptr;
		
		ptr += products.size() * sizeof(giab_Product);
		 
		for (std::size_t i = 0; i < count; ++i)
		{	
			event->products[i].productId = ptr;
			strcpy(ptr, products[i].productId.c_str());
			ptr += products[i].productId.size() + 1;
		
			event->products[i].title = ptr;
			strcpy(ptr, products[i].title.c_str());
			ptr += products[i].title.size() + 1;
			
			event->products[i].description = ptr;
			strcpy(ptr, products[i].description.c_str());
			ptr += products[i].description.size() + 1;
		
			event->products[i].price = ptr;
			strcpy(ptr, products[i].price.c_str());
			ptr += products[i].price.size() + 1;
		}
		gevent_EnqueueEvent(gid_, callback_s, GIAB_PRODUCTS_COMPLETE_EVENT, event, 1, this);
	}
	
	void onProductsError(jstring jIap, jstring jerror)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *iap = env->GetStringUTFChars(jIap, NULL);
		const char *value = env->GetStringUTFChars(jerror, NULL);
		giab_DoubleEvent *event = (giab_DoubleEvent*)gevent_CreateEventStruct2(
			sizeof(giab_DoubleEvent),
			offsetof(giab_DoubleEvent, iap), iap,
			offsetof(giab_DoubleEvent, value), value);
		env->ReleaseStringUTFChars(jIap, iap);
		env->ReleaseStringUTFChars(jerror, value);
		gevent_EnqueueEvent(gid_, callback_s, GIAB_PRODUCTS_ERROR_EVENT, event, 1, this);
	}
	
	void onRestoreComplete(jstring jIap)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *iap = env->GetStringUTFChars(jIap, NULL);
		giab_SimpleEvent *event = (giab_SimpleEvent*)gevent_CreateEventStruct1(
			sizeof(giab_SimpleEvent),
			offsetof(giab_SimpleEvent, iap), iap);
		env->ReleaseStringUTFChars(jIap, iap);
		gevent_EnqueueEvent(gid_, callback_s, GIAB_RESTORE_COMPLETE_EVENT, event, 1, this);
	}
	
	void onRestoreError(jstring jIap, jstring jerror)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *iap = env->GetStringUTFChars(jIap, NULL);
		const char *value = env->GetStringUTFChars(jerror, NULL);
		giab_DoubleEvent *event = (giab_DoubleEvent*)gevent_CreateEventStruct2(
			sizeof(giab_DoubleEvent),
			offsetof(giab_DoubleEvent, iap), iap,
			offsetof(giab_DoubleEvent, value), value);
		env->ReleaseStringUTFChars(jIap, iap);
		env->ReleaseStringUTFChars(jerror, value);
		gevent_EnqueueEvent(gid_, callback_s, GIAB_RESTORE_ERROR_EVENT, event, 1, this);
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
		((GIab*)udata)->callback(type, event);
	}

	void callback(int type, void *event)
	{
		callbackList_.dispatchEvent(type, event);
	}

private:
	gevent_CallbackList callbackList_;

private:
	jclass cls_;
	jclass clsBundle;
	jclass clsSparse;
	g_id gid_;
	std::vector<Product> products;
	std::vector<giab_SimpleParam> stores;
};

extern "C" {

void Java_com_giderosmobile_android_plugins_iab_Iab_onAvailable(JNIEnv *env, jclass clz, jstring jiap, jlong data)
{
	((GIab*)data)->onAvailable(jiap);
}

void Java_com_giderosmobile_android_plugins_iab_Iab_onNotAvailable(JNIEnv *env, jclass clz, jstring jiap, jlong data)
{
	((GIab*)data)->onNotAvailable(jiap);
}

void Java_com_giderosmobile_android_plugins_iab_Iab_onProductsComplete(JNIEnv *env, jclass clz, jstring jiap, jobject products, jlong data)
{
	((GIab*)data)->onProductsComplete(jiap, products);
}

void Java_com_giderosmobile_android_plugins_iab_Iab_onProductsError(JNIEnv *env, jclass clz, jstring jiap, jstring error, jlong data)
{
	((GIab*)data)->onProductsError(jiap, error);
}

void Java_com_giderosmobile_android_plugins_iab_Iab_onPurchaseComplete(JNIEnv *env, jclass clz, jstring jiap, jstring jproduct, jstring jreceipt, jlong data)
{
	((GIab*)data)->onPurchaseComplete(jiap, jproduct, jreceipt);
}

void Java_com_giderosmobile_android_plugins_iab_Iab_onPurchaseError(JNIEnv *env, jclass clz, jstring jiap, jstring error, jlong data)
{
	((GIab*)data)->onPurchaseError(jiap, error);
}

void Java_com_giderosmobile_android_plugins_iab_Iab_onRestoreComplete(JNIEnv *env, jclass clz, jstring jiap, jlong data)
{
	((GIab*)data)->onRestoreComplete(jiap);
}

void Java_com_giderosmobile_android_plugins_iab_Iab_onRestoreError(JNIEnv *env, jclass clz, jstring jiap, jstring error, jlong data)
{
	((GIab*)data)->onRestoreError(jiap, error);
}


}

static GIab *s_giab = NULL;

extern "C" {

void giab_init()
{
	s_giab = new GIab;
}

void giab_cleanup()
{
	if(s_giab)
	{
		delete s_giab;
		s_giab = NULL;
	}
}

giab_SimpleParam* giab_detectStores(giab_SimpleParam *params){
	if(s_giab)
	{
		return s_giab->detectStores(params);
	}
	return NULL;
}

void giab_initialize(const char *iab)
{
	if(s_giab)
	{
		s_giab->initialize(iab);
	}
}

void giab_destroy(const char *iab)
{
	if(s_giab)
	{
		s_giab->destroy(iab);
	}
}

void giab_setup(const char *iab, giab_Parameter *params)
{
	if(s_giab)
	{
		s_giab->setup(iab, params);
	}
}

void giab_setProducts(const char *iab, giab_DoubleEvent *params)
{
	if(s_giab)
	{
		s_giab->setProducts(iab, params);
	}
}

void giab_setConsumables(const char *iab, const char * const *params)
{
	if(s_giab)
	{
		s_giab->setConsumables(iab, params);
	}
}

void giab_check(const char *iab)
{
	if(s_giab)
	{
		s_giab->check(iab);
	}
}

void giab_request(const char *iab)
{
	if(s_giab)
	{
		s_giab->request(iab);
	}
}

void giab_purchase(const char *iab, const char *productId)
{
	if(s_giab)
	{
		s_giab->purchase(iab, productId);
	}
}

void giab_restore(const char *iab)
{
	if(s_giab)
	{
		s_giab->restore(iab);
	}
}

g_id giab_addCallback(gevent_Callback callback, void *udata)
{
	if(s_giab)
	{
		return s_giab->addCallback(callback, udata);
	}
}

void giab_removeCallback(gevent_Callback callback, void *udata)
{
	if(s_giab)
	{
		s_giab->removeCallback(callback, udata);
	}
}

void giab_removeCallbackWithGid(g_id gid)
{
	if(s_giab)
	{
		s_giab->removeCallbackWithGid(gid);
	}
}

}
