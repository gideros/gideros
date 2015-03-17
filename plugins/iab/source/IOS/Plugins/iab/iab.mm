#include "iab.h"
#import "IabIos.h"

class GIab
{
public:
	GIab()
	{
		gid_ = g_NextId();
        i = nil;
	}

	~GIab()
	{
		gevent_RemoveEventsWithGid(gid_);
	}
	
	void initialize(const char *iab)
	{
		if(i == nil)
            i = [[IabIos alloc] init];
	}
	
	void destroy(const char *iab)
	{
		[i deinit];
        i = nil;
	}
	
	giab_SimpleParam* detectStores(giab_SimpleParam *params)
	{
		stores.clear();
		giab_SimpleParam param = {"ios"};
		stores.push_back(param);

		giab_SimpleParam param2 = {""};
		stores.push_back(param2);
		
		return &stores[0];
	}
	
	void setup(const char *iab, giab_Parameter *params)
	{
		
	}
	
	void check(const char *iab)
	{
		[i check];
	}
	
	void setProducts(const char *iab, giab_DoubleEvent *params){
        NSMutableDictionary* productIdentifiers = [NSMutableDictionary dictionary];
		while (params->iap)
		{
			[productIdentifiers setObject:[NSString stringWithUTF8String:params->value] forKey:[NSString stringWithUTF8String:params->iap]];
			params++;
		}
        [i setProducts:productIdentifiers];
	}
	
	void setConsumables(const char *iab, const char * const *params){
	}
	
	void request(const char *iab){
        [i request];
	}
	
	void purchase(const char *iab, const char *productId)
	{
		[i purchase:[NSString stringWithUTF8String:productId]];
	}
	
	void restore(const char *iab)
	{
		[i restore];
	}
	
	void map2products(Product *product)
	{
		products.clear();
        
        while (!product->productId.empty()){
            Product gprod = {product->productId, product->title, product->description, product->price};
            products.push_back(gprod);
            ++product;
        }
	}
	
	void onAvailable(const char *iap)
	{
		giab_SimpleEvent *event = (giab_SimpleEvent*)gevent_CreateEventStruct1(
			sizeof(giab_SimpleEvent),
			offsetof(giab_SimpleEvent, iap), iap);
		gevent_EnqueueEvent(gid_, callback_s, GIAB_AVAILABLE_EVENT, event, 1, this);
	}
	
	void onNotAvailable(const char *iap)
	{
		giab_SimpleEvent *event = (giab_SimpleEvent*)gevent_CreateEventStruct1(
			sizeof(giab_SimpleEvent),
			offsetof(giab_SimpleEvent, iap), iap);
		gevent_EnqueueEvent(gid_, callback_s, GIAB_NOT_AVAILABLE_EVENT, event, 1, this);
	}
	
	void onPurchaseComplete(const char* iap, const char *productId, const char* receiptId)
	{
		giab_Purchase *event = (giab_Purchase*)gevent_CreateEventStruct3(
			sizeof(giab_Purchase),
			offsetof(giab_Purchase, iap), iap,
			offsetof(giab_Purchase, productId), productId,
			offsetof(giab_Purchase, receiptId), receiptId);
    
		gevent_EnqueueEvent(gid_, callback_s, GIAB_PURCHASE_COMPLETE_EVENT, event, 1, this);
	}
	
	void onPurchaseError(const char *iap, const char *value)
	{
		giab_DoubleEvent *event = (giab_DoubleEvent*)gevent_CreateEventStruct2(
			sizeof(giab_DoubleEvent),
			offsetof(giab_DoubleEvent, iap), iap,
			offsetof(giab_DoubleEvent, value), value);
		gevent_EnqueueEvent(gid_, callback_s, GIAB_PURCHASE_ERROR_EVENT, event, 1, this);
	}
	
	void onProductsComplete(const char *iab, Product *product)
	{
		this->map2products(product);
		
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
		std::string iap(iab);
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
	
	void onProductsError(const char *iap, const char *value)
	{
		giab_DoubleEvent *event = (giab_DoubleEvent*)gevent_CreateEventStruct2(
			sizeof(giab_DoubleEvent),
			offsetof(giab_DoubleEvent, iap), iap,
			offsetof(giab_DoubleEvent, value), value);
		gevent_EnqueueEvent(gid_, callback_s, GIAB_PRODUCTS_ERROR_EVENT, event, 1, this);
	}
	
	void onRestoreComplete(const char *iap)
	{
		giab_SimpleEvent *event = (giab_SimpleEvent*)gevent_CreateEventStruct1(
			sizeof(giab_SimpleEvent),
			offsetof(giab_SimpleEvent, iap), iap);
		gevent_EnqueueEvent(gid_, callback_s, GIAB_RESTORE_COMPLETE_EVENT, event, 1, this);
	}
	
	void onRestoreError(const char *iap, const char *value)
	{
		giab_DoubleEvent *event = (giab_DoubleEvent*)gevent_CreateEventStruct2(
			sizeof(giab_DoubleEvent),
			offsetof(giab_DoubleEvent, iap), iap,
			offsetof(giab_DoubleEvent, value), value);
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
	g_id gid_;
    IabIos *i;
	std::vector<Product> products;
	std::vector<giab_SimpleParam> stores;
};

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
    
void giab_onAvailable(const char *iap){
    if(s_giab)
    {
        s_giab->onAvailable(iap);
    }
}
    
void giab_onNotAvailable(const char *iap){
    if(s_giab)
    {
        s_giab->onNotAvailable(iap);
    }
}
    
void giab_onPurchaseComplete(const char* iap, const char *productId, const char* receiptId){
    if(s_giab)
    {
        s_giab->onPurchaseComplete(iap, productId, receiptId);
    }
}
    
void giab_onPurchaseError(const char *iap, const char *value){
    if(s_giab)
    {
        s_giab->onPurchaseError(iap, value);
    }
}
    
void giab_onProductsComplete(const char *iab, Product *product){
    if(s_giab)
    {
        s_giab->onProductsComplete(iab, product);
    }
}
    
void giab_onProductsError(const char *iap, const char *value){
    if(s_giab)
    {
        s_giab->onProductsError(iap, value);
    }
}
    
void giab_onRestoreComplete(const char *iap){
    if(s_giab)
    {
        s_giab->onRestoreComplete(iap);
    }
}
    
void giab_onRestoreError(const char *iap, const char *value){
    if(s_giab)
    {
        s_giab->onRestoreError(iap, value);
    }
}

g_id giab_addCallback(gevent_Callback callback, void *udata)
{
	if(s_giab)
	{
		return s_giab->addCallback(callback, udata);
	}
    return NULL;
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
