#include "iab.h"

#include <wrl.h>
#include <collection.h>
#include <concrt.h>
#include <algorithm>
#include <windows.foundation.collections.h>
//#include <Windows.ApplicationModel.store.h>
#include <windows.services.store.h>
#include <glog.h>
#include <ppltasks.h>
#include "giderosapi.h"

using namespace Concurrency;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Services::Store;

std::wstring utf8_ws(const char* str);
std::string utf8_us(const wchar_t* str);

std::string us(String^ ref) {
	return utf8_us(ref->Data());
}

String^ ws(std::string u) {
	return ref new String(utf8_ws(u.c_str()).c_str());
}

class GIab
{
	StoreContext^ ctx=nullptr;
	std::map<std::string, std::string> storeId;
	std::map<std::string, std::string> internalId;
public:
	GIab()
	{
		gid_ = g_NextId();
	}

	~GIab()
	{
		gevent_RemoveEventsWithGid(gid_);
	}
	
	void initialize(const char *iab)
	{
		storeId.clear();
		internalId.clear();
		ctx = StoreContext::GetDefault();
	}
	
	void destroy(const char *iab)
	{
		ctx = nullptr;
	}
	
	giab_SimpleParam* detectStores(giab_SimpleParam *params)
	{
		stores.clear();
		giab_SimpleParam param = {"uwp"};
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
#if 0
		gdr_dispatchUi([&]()
		{
			create_task(ctx->GetAppLicenseAsync()).then([=](task<StoreAppLicense^> previousTask)
			{
				StoreAppLicense^ license = previousTask.get();
				onAvailable(iab);
			});
		}, true);
#else
		onAvailable(iab);
#endif
	}
	
	void setProducts(const char *iab, giab_DoubleEvent *params){
		storeId.clear();
		internalId.clear();
		while (params->iap)
		{
			storeId[params->iap] = params->value;
			internalId[params->value] = params->iap;
			params++;
		}
	}
	
	void setConsumables(const char *iab, const char * const *params){
	}
	
	void request(const char *iab){
		gdr_dispatchUi([&]()
		{
			Vector<String^> ^pType = ref new Vector<String^>;
			pType->Append("Durable");
			pType->Append("Consumable");
			pType->Append("UnmanagedConsumable");

			create_task(ctx->GetAssociatedStoreProductsAsync(pType)).then([=](task<StoreProductQueryResult^> previousTask)
			{
				StoreProductQueryResult^ products = previousTask.get();
				std::vector<Product> pl;
				if (products->Products==nullptr) {
					onProductsError(iab, us(products->ExtendedError.ToString()).c_str());
				}
				else
				{
					auto ip=products->Products->First();
					while (ip->HasCurrent) {
						auto sk = ip->Current->Key;
						auto sp = ip->Current->Value;
						auto id = internalId[us(sp->StoreId)];
						if (!id.empty()) {
							Product p;
							p.description = us(sp->Description);
							p.productId = id;
							p.price = us(sp->Price->FormattedPrice);
							p.title = us(sp->Title);
							pl.push_back(p);
						}
						ip->MoveNext();
					}
					Product p;
					pl.push_back(p);
					onProductsComplete(iab, &pl[0]);
				}
			});
		}, true);
	}
	
	void purchase(const char *iab, const char *productId)
	{
		std::string sid = storeId[productId];
		if (sid.empty())
		{
			onPurchaseError(iab, "No such product ID");
		}
		else
			gdr_dispatchUi([&]()
			{
				create_task(ctx->RequestPurchaseAsync(ws(sid))).then([=](task<StorePurchaseResult^> previousTask)
				{
					StorePurchaseResult^ result = previousTask.get();
					if ((result->Status == StorePurchaseStatus::Succeeded) ||
						(result->Status == StorePurchaseStatus::AlreadyPurchased)) {
						onPurchaseComplete(iab, productId,"");
					}
					else
					{
						onPurchaseError(iab, us(result->ExtendedError.ToString()).c_str());
					}
				});
			}, true);
	}
	
	void restore(const char *iab)
	{
		gdr_dispatchUi([&]()
		{
			Vector<String^>^ pType = ref new Vector<String^>;
			pType->Append("Durable");
			pType->Append("Consumable");
			pType->Append("UnmanagedConsumable");

			create_task(ctx->GetUserCollectionAsync(pType)).then([=](task<StoreProductQueryResult^> previousTask)
			{
				StoreProductQueryResult^ products = previousTask.get();
				Product p;
				if (products->Products == nullptr) {
					onRestoreError(iab, us(products->ExtendedError.ToString()).c_str());
				}
				else
				{
					auto ip = products->Products->First();
					while (ip->HasCurrent) {
						auto sk = ip->Current->Key;
						auto sp = ip->Current->Value;
						onPurchaseComplete(iab, internalId[us(sp->StoreId)].c_str(),"");
						ip->MoveNext();
					}
					onRestoreComplete(iab);
				}
			});
		}, true);
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
