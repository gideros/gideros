#include "ggooglebilling.h"
#include "gideros.h"
#include <map>
#include <string>
#include <vector>
#include <glog.h>

// some Lua helper functions
#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif

static void luaL_newweaktable(lua_State *L, const char *mode)
{
	lua_newtable(L);			// create table for instance list
	lua_pushstring(L, mode);
	lua_setfield(L, -2, "__mode");	  // set as weak-value table
	lua_pushvalue(L, -1);             // duplicate table
	lua_setmetatable(L, -2);          // set itself as metatable
}

static void luaL_rawgetptr(lua_State *L, int idx, void *ptr)
{
	idx = abs_index(L, idx);
	lua_pushlightuserdata(L, ptr);
	lua_rawget(L, idx);
}

static void luaL_rawsetptr(lua_State *L, int idx, void *ptr)
{
	idx = abs_index(L, idx);
	lua_pushlightuserdata(L, ptr);
	lua_insert(L, -2);
	lua_rawset(L, idx);
}

static const char *CHECK_BILLING_SUPPORTED_COMPLETE = "checkBillingSupportedComplete";
static const char *REQUEST_PURCHASE_COMPLETE = "requestPurchaseComplete";
static const char *RESTORE_TRANSACTIONS_COMPLETE = "restoreTransactionsComplete";
static const char *CONFIRM_NOTIFICATION_COMPLETE = "confirmNotificationComplete";
static const char *PURCHASE_STATE_CHANGE = "purchaseStateChange";

static const char *OK = "ok";
static const char *USER_CANCELED = "userCanceled";
static const char *SERVICE_UNAVAILABLE = "serviceUnavailable";
static const char *BILLING_UNAVAILABLE = "billingUnavailable";
static const char *ITEM_UNAVAILABLE = "itemUnavailable";
static const char *DEVELOPER_ERROR = "developerError";
static const char *ERROR = "error";

static const char *PURCHASED = "purchased";
static const char *CANCELED = "canceled";
static const char *REFUNDED = "refunded";
static const char *EXPIRED = "expired";

static const char *INAPP = "inapp";
static const char *SUBS = "subs";

const char *responseCode2str(int responseCode)
{
	switch (responseCode)
	{
	case GGOOGLEBILLING_RESULT_OK:
		return OK;
	case GGOOGLEBILLING_RESULT_USER_CANCELED:
		return USER_CANCELED;
	case GGOOGLEBILLING_RESULT_SERVICE_UNAVAILABLE:
		return SERVICE_UNAVAILABLE;
	case GGOOGLEBILLING_RESULT_BILLING_UNAVAILABLE:
		return BILLING_UNAVAILABLE;
	case GGOOGLEBILLING_RESULT_ITEM_UNAVAILABLE:
		return ITEM_UNAVAILABLE;
	case GGOOGLEBILLING_RESULT_DEVELOPER_ERROR:
		return DEVELOPER_ERROR;
	case GGOOGLEBILLING_RESULT_ERROR:
		return ERROR;
	};
	
	return "undefined";
}

const char *purchaseState2str(int purchaseState)
{
	switch (purchaseState)
	{
	case GGOOGLEBILLING_PURCHASED:
		return PURCHASED;
	case GGOOGLEBILLING_CANCELED:
		return CANCELED;
	case GGOOGLEBILLING_REFUNDED:
		return REFUNDED;
	case GGOOGLEBILLING_EXPIRED:
		return EXPIRED;
	};
	
	return "undefined";
}


static char keyWeak = ' ';

static char *time2str(time_t t, char *str)
{
    strftime(str, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
    return str;
}

class GoogleBilling : public GEventDispatcherProxy
{
public:
    GoogleBilling(lua_State *L) : L(L)
    {
        ggooglebilling_init();
		ggooglebilling_addCallback(callback_s, this);		
    }
    
    ~GoogleBilling()
    {
		ggooglebilling_removeCallback(callback_s, this);
		ggooglebilling_cleanup();
    }
	
	void setPublicKey(const char *publicKey)
	{
		ggooglebilling_setPublicKey(publicKey);
	}
	
	void setApiVersion(int apiVersion)
	{
		ggooglebilling_setApiVersion(apiVersion);
	}

	bool checkBillingSupported(const char *productType)
	{
		return ggooglebilling_checkBillingSupported(productType);
	}

	bool requestPurchase(const char *productId, const char* productType, const char *developerPayload)
	{
		return ggooglebilling_requestPurchase(productId, productType, developerPayload);
	}

	bool confirmNotification(const char *notificationId)
	{
		return ggooglebilling_confirmNotification(notificationId);
	}

	bool restoreTransactions()
	{
		return ggooglebilling_restoreTransactions();
	}

private:
	static void callback_s(int type, void *event, void *udata)
	{
		static_cast<GoogleBilling*>(udata)->callback(type, event);
	}
	
	void callback(int type, void *event)
	{
        dispatchEvent(type, event);
	}
	
	void dispatchEvent(int type, void *event)
	{
        luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
        luaL_rawgetptr(L, -1, this);
		
        if (lua_isnil(L, -1))
        {
            lua_pop(L, 2);
            return;
        }
        
        lua_getfield(L, -1, "dispatchEvent");
		
        lua_pushvalue(L, -2);
        
        lua_getglobal(L, "Event");
        lua_getfield(L, -1, "new");
        lua_remove(L, -2);
        
        switch (type)
        {
            case GGOOGLEBILLING_CHECK_BILLING_SUPPORTED_COMPLETE_EVENT:
                lua_pushstring(L, CHECK_BILLING_SUPPORTED_COMPLETE);
                break;
            case GGOOGLEBILLING_PURCHASE_STATE_CHANGE_EVENT:
                lua_pushstring(L, PURCHASE_STATE_CHANGE);
                break;
            case GGOOGLEBILLING_REQUEST_PURCHASE_COMPLETE_EVENT:
                lua_pushstring(L, REQUEST_PURCHASE_COMPLETE);
                break;
            case GGOOGLEBILLING_RESTORE_TRANSACTIONS_COMPLETE_EVENT:
                lua_pushstring(L, RESTORE_TRANSACTIONS_COMPLETE);
                break;
            case GGOOGLEBILLING_CONFIRM_NOTIFICATION_COMPLETE_EVENT:
                lua_pushstring(L, CONFIRM_NOTIFICATION_COMPLETE);
                break;
        }

        lua_call(L, 1, 1);

        if (type == GGOOGLEBILLING_CHECK_BILLING_SUPPORTED_COMPLETE_EVENT)
        {
            ggooglebilling_CheckBillingSupportedCompleteEvent *event2 = (ggooglebilling_CheckBillingSupportedCompleteEvent*)event;
            
			lua_pushstring(L, responseCode2str(event2->responseCode));
			lua_setfield(L, -2, "responseCode");
			
			if (event2->productType)
			{
				lua_pushstring(L, event2->productType);
				lua_setfield(L, -2, "productType");
			}
        }
		else if (type == GGOOGLEBILLING_PURCHASE_STATE_CHANGE_EVENT)
		{
            ggooglebilling_PurchaseStateChangeEvent *event2 = (ggooglebilling_PurchaseStateChangeEvent*)event;
		
			lua_pushstring(L, purchaseState2str(event2->purchaseState));
			lua_setfield(L, -2, "purchaseState");
			
			lua_pushstring(L, event2->productId);
			lua_setfield(L, -2, "productId");

			if (event2->notificationId)
			{
				lua_pushstring(L, event2->notificationId);
				lua_setfield(L, -2, "notificationId");
			}

			char purchaseTime[20];
			time2str(event2->purchaseTime, purchaseTime);
			lua_pushstring(L, purchaseTime);
			lua_setfield(L, -2, "purchaseTime");
			
			if (event2->developerPayload)
			{
				lua_pushstring(L, event2->developerPayload);
				lua_setfield(L, -2, "developerPayload");
			}
		}
		else if (type == GGOOGLEBILLING_REQUEST_PURCHASE_COMPLETE_EVENT)
		{
            ggooglebilling_RequestPurchaseCompleteEvent *event2 = (ggooglebilling_RequestPurchaseCompleteEvent*)event;
		
			lua_pushstring(L, responseCode2str(event2->responseCode));
			lua_setfield(L, -2, "responseCode");
		
			lua_pushstring(L, event2->productId);
			lua_setfield(L, -2, "productId");
		
			if (event2->productType)
			{
				lua_pushstring(L, event2->productType);
				lua_setfield(L, -2, "productType");
			}

			if (event2->developerPayload)
			{
				lua_pushstring(L, event2->developerPayload);
				lua_setfield(L, -2, "developerPayload");
			}		
		}
		else if (type == GGOOGLEBILLING_RESTORE_TRANSACTIONS_COMPLETE_EVENT)
		{
            ggooglebilling_RestoreTransactionsCompleteEvent *event2 = (ggooglebilling_RestoreTransactionsCompleteEvent*)event;
		
			lua_pushstring(L, responseCode2str(event2->responseCode));
			lua_setfield(L, -2, "responseCode");
		}
		else if (type == GGOOGLEBILLING_CONFIRM_NOTIFICATION_COMPLETE_EVENT)
		{
            ggooglebilling_ConfirmNotificationCompleteEvent *event2 = (ggooglebilling_ConfirmNotificationCompleteEvent*)event;

			lua_pushstring(L, responseCode2str(event2->responseCode));
			lua_setfield(L, -2, "responseCode");

			lua_pushstring(L, event2->notificationId);
			lua_setfield(L, -2, "notificationId");
		}

		lua_call(L, 2, 0);
		
		lua_pop(L, 2);
	}

private:
    lua_State *L;
    bool initialized_;
};

static int destruct(lua_State* L)
{
	void *ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	GoogleBilling *googlebilling = static_cast<GoogleBilling*>(object->proxy());
	
	googlebilling->unref();
	
	return 0;
}

static GoogleBilling *getInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "GoogleBilling", index));
	GoogleBilling *googlebilling = static_cast<GoogleBilling*>(object->proxy());
    
	return googlebilling;
}

static int setPublicKey(lua_State *L)
{
    GoogleBilling *googlebilling = getInstance(L, 1);
    
    const char *publicKey = luaL_checkstring(L, 2);
    
    googlebilling->setPublicKey(publicKey);
    
    return 0;
}

static int setApiVersion(lua_State *L)
{
    GoogleBilling *googlebilling = getInstance(L, 1);
    
    int apiVersion = luaL_checkinteger(L, 2);
    
    googlebilling->setApiVersion(apiVersion);
    
    return 0;
}

static int checkBillingSupported(lua_State *L)
{
    GoogleBilling *googlebilling = getInstance(L, 1);

    const char *productType = luaL_optstring(L, 2, NULL);

    lua_pushboolean(L, googlebilling->checkBillingSupported(productType));
    
    return 1;
}

static int requestPurchase(lua_State *L)
{
    GoogleBilling *googlebilling = getInstance(L, 1);
        
    const char *productId = luaL_checkstring(L, 2);
    const char *productType = luaL_optstring(L, 3, NULL);
    const char *developerPayload = luaL_optstring(L, 4, NULL);

    lua_pushboolean(L, googlebilling->requestPurchase(productId, productType, developerPayload));
    
    return 1;
}

static int confirmNotification(lua_State *L)
{
    GoogleBilling *googlebilling = getInstance(L, 1);
        
    const char *notificationId = luaL_checkstring(L, 2);

    lua_pushboolean(L, googlebilling->confirmNotification(notificationId));
    
    return 1;
}

static int restoreTransactions(lua_State *L)
{
    GoogleBilling *googlebilling = getInstance(L, 1);

    lua_pushboolean(L, googlebilling->restoreTransactions());
    
    return 1;
}

static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
        {"setPublicKey", setPublicKey},
        {"setApiVersion", setApiVersion},
        {"checkBillingSupported", checkBillingSupported},
        {"requestPurchase", requestPurchase},
        {"confirmNotification", confirmNotification},
        {"restoreTransactions", restoreTransactions},
		{NULL, NULL},
	};
    
    g_createClass(L, "GoogleBilling", "EventDispatcher", NULL, destruct, functionlist);
    
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	
	lua_getglobal(L, "GoogleBilling");
	lua_pushstring(L, OK);
	lua_setfield(L, -2, "OK");
	lua_pushstring(L, USER_CANCELED);
	lua_setfield(L, -2, "USER_CANCELED");
	lua_pushstring(L, SERVICE_UNAVAILABLE);
	lua_setfield(L, -2, "SERVICE_UNAVAILABLE");
	lua_pushstring(L, BILLING_UNAVAILABLE);
	lua_setfield(L, -2, "BILLING_UNAVAILABLE");
	lua_pushstring(L, ITEM_UNAVAILABLE);
	lua_setfield(L, -2, "ITEM_UNAVAILABLE");
	lua_pushstring(L, DEVELOPER_ERROR);
	lua_setfield(L, -2, "DEVELOPER_ERROR");
	lua_pushstring(L, ERROR);
	lua_setfield(L, -2, "ERROR");
	lua_pushstring(L, PURCHASED);
	lua_setfield(L, -2, "PURCHASED");
	lua_pushstring(L, CANCELED);
	lua_setfield(L, -2, "CANCELED");
	lua_pushstring(L, REFUNDED);
	lua_setfield(L, -2, "REFUNDED");
	lua_pushstring(L, EXPIRED);
	lua_setfield(L, -2, "EXPIRED");
	lua_pushstring(L, INAPP);
	lua_setfield(L, -2, "INAPP");
	lua_pushstring(L, SUBS);
	lua_setfield(L, -2, "SUBS");
	lua_pop(L, 1);
    
	lua_getglobal(L, "Event");
	lua_pushstring(L, CHECK_BILLING_SUPPORTED_COMPLETE);
	lua_setfield(L, -2, "CHECK_BILLING_SUPPORTED_COMPLETE");
	lua_pushstring(L, PURCHASE_STATE_CHANGE);
	lua_setfield(L, -2, "PURCHASE_STATE_CHANGE");
	lua_pushstring(L, REQUEST_PURCHASE_COMPLETE);
	lua_setfield(L, -2, "REQUEST_PURCHASE_COMPLETE");
	lua_pushstring(L, RESTORE_TRANSACTIONS_COMPLETE);
	lua_setfield(L, -2, "RESTORE_TRANSACTIONS_COMPLETE");
	lua_pushstring(L, CONFIRM_NOTIFICATION_COMPLETE);
	lua_setfield(L, -2, "CONFIRM_NOTIFICATION_COMPLETE");
	lua_pop(L, 1);
	
    GoogleBilling *googlebilling = new GoogleBilling(L);
	g_pushInstance(L, "GoogleBilling", googlebilling->object());
    
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, googlebilling);
	lua_pop(L, 1);
    
	lua_pushvalue(L, -1);
	lua_setglobal(L, "googlebilling");
    
    return 1;
}
    
static void g_initializePlugin(lua_State *L)
{
    lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "googlebilling");
	
	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
    
}

REGISTER_PLUGIN("Google Billing", "1.0")
