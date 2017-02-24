/*

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2012 Gideros Mobile 

*/

#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"

#import <StoreKit/StoreKit.h>

static const char KEY_OBJECTS = ' ';

static const char* REQUEST_PRODUCTS_COMPLETE = "requestProductsComplete";
static const char* TRANSACTION = "transaction";
static const char* RESTORE_TRANSACTIONS_COMPLETE = "restoreTransactionsComplete";

static const char* PURCHASED = "purchased";
static const char* RESTORED = "restored";
static const char* FAILED = "failed";

static void dispatchEvent(lua_State* L, const char* type,
                          NSError* error,
						  NSArray* products,
						  NSArray* invalidProductIdentifiers,
						  SKPaymentTransaction* transaction)
{
	lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
	lua_rawget(L, LUA_REGISTRYINDEX);
	
	lua_rawgeti(L, -1, 1);

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
	lua_pushstring(L, type);
	lua_call(L, 1, 1);	

    if (error)
    {
        lua_pushinteger(L, error.code);
        lua_setfield(L, -2, "errorCode");
        
        lua_pushstring(L, [error.localizedDescription UTF8String]);
        lua_setfield(L, -2, "errorDescription");        
    }
    
	if (products)
	{
		lua_newtable(L);
		for(int i = 0; i < [products count]; ++i)
		{
			SKProduct* product = [products objectAtIndex:i];
						
			lua_newtable(L);
			
			lua_pushstring(L, [product.localizedTitle UTF8String]);
			lua_setfield(L, -2, "title");

			lua_pushstring(L, [product.localizedDescription UTF8String]);
			lua_setfield(L, -2, "description");

			lua_pushnumber(L, [product.price doubleValue]);
			lua_setfield(L, -2, "price");

			lua_pushstring(L, [product.productIdentifier UTF8String]);
			lua_setfield(L, -2, "productIdentifier");
            
            
            NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
            [numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
            [numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
            [numberFormatter setLocale:product.priceLocale];
            NSString *formattedPrice = [numberFormatter stringFromNumber:product.price];
            lua_pushstring(L, [formattedPrice UTF8String]);
            lua_setfield(L, -2, "currencyString");
            
			lua_rawseti(L, -2, i + 1);
		}
		lua_setfield(L, -2, "products");
	}

	if (invalidProductIdentifiers)
	{
		lua_newtable(L);
		for(int i = 0; i < [invalidProductIdentifiers count]; ++i)
		{
			NSString* invalidProduct = [invalidProductIdentifiers objectAtIndex:i];
			lua_pushstring(L, [invalidProduct UTF8String]);
			lua_rawseti(L, -2, i + 1);
			
		}
		lua_setfield(L, -2, "invalidProductIdentifiers");
	}
	
	if (transaction)
	{
		if (transaction.error)
		{
			lua_pushinteger(L, transaction.error.code);
			lua_setfield(L, -2, "errorCode");
			
			lua_pushstring(L, [transaction.error.localizedDescription UTF8String]);
			lua_setfield(L, -2, "errorDescription");			
		}		
		
		if (transaction.payment)
		{
			lua_newtable(L);
			
			lua_pushstring(L, [transaction.payment.productIdentifier UTF8String]);
			lua_setfield(L, -2, "productIdentifier");
			
			lua_pushinteger(L, transaction.payment.quantity);
			lua_setfield(L, -2, "quantity");
						
			lua_setfield(L, -2, "payment");		
		}
		
		lua_newtable(L);
		
		lua_pushlightuserdata(L, transaction);
		lua_setfield(L, -2, "__userdata");
		
		switch (transaction.transactionState)
		{
			case SKPaymentTransactionStatePurchased:
				lua_pushstring(L, PURCHASED);
				lua_setfield(L, -2, "state");
				break;
			case SKPaymentTransactionStateFailed:
				lua_pushstring(L, FAILED);
				lua_setfield(L, -2, "state");
				break;
			case SKPaymentTransactionStateRestored:
				lua_pushstring(L, RESTORED);
				lua_setfield(L, -2, "state");
				break;
            default:
                break;
        }
		
		if (transaction.transactionState == SKPaymentTransactionStatePurchased ||
			transaction.transactionState == SKPaymentTransactionStateRestored)
		{
			lua_pushstring(L, [transaction.transactionIdentifier UTF8String]);
			lua_setfield(L, -2, "identifier");
		}
		
		if (transaction.transactionState == SKPaymentTransactionStatePurchased)
		{
			lua_pushlstring(L, (const char*)transaction.transactionReceipt.bytes, transaction.transactionReceipt.length);
			lua_setfield(L, -2, "receipt");
		}

		if (transaction.transactionState == SKPaymentTransactionStatePurchased ||
			transaction.transactionState == SKPaymentTransactionStateRestored)
		{
			NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
			[dateFormatter setDateFormat: @"yyyy-MM-dd HH:mm:ss"];
			NSString* transactionDate = [dateFormatter stringFromDate:transaction.transactionDate];
			lua_pushstring(L, [transactionDate UTF8String]);
			lua_setfield(L, -2, "date");			
			[dateFormatter release];
		}
		lua_setfield(L, -2, "transaction");
		
		if (transaction.transactionState == SKPaymentTransactionStateRestored)
		{
			SKPaymentTransaction* originalTransaction = transaction.originalTransaction;
			
			lua_newtable(L);
			
			lua_pushstring(L, [originalTransaction.transactionIdentifier UTF8String]);
			lua_setfield(L, -2, "identifier");

			NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
			[dateFormatter setDateFormat: @"yyyy-MM-dd HH:mm:ss"];
			NSString* transactionDate = [dateFormatter stringFromDate:originalTransaction.transactionDate];
			lua_pushstring(L, [transactionDate UTF8String]);
			lua_setfield(L, -2, "date");			
			[dateFormatter release];

			lua_setfield(L, -2, "originalTransaction");
		}
    }

	if (lua_pcall(L, 2, 0, 0) != 0)
		g_error(L, lua_tostring(L, -1));
	
	lua_pop(L, 2);
}

static std::set<SKRequest*> s_requests;

@interface StoreKitHelper : NSObject<SKProductsRequestDelegate, SKPaymentTransactionObserver>
{
	lua_State* L;
}
@end

@implementation StoreKitHelper


- (id)initWithLuaState:(lua_State *)theL
{
	if (self = [super init])
	{
		L = theL;
	}
	
	return self;
}

- (void)requestDidFinish:(SKRequest *)request
{
    s_requests.erase(request);
    [request release];
}

- (void)request:(SKRequest *)request didFailWithError:(NSError *)error
{
	dispatchEvent(L, REQUEST_PRODUCTS_COMPLETE, error, NULL, NULL, NULL);

    s_requests.erase(request);
    [request release];
}

- (void)productsRequest:(SKProductsRequest*)request didReceiveResponse:(SKProductsResponse*)response
{
    if (response.products == nil || response.invalidProductIdentifiers == nil)
    {
        NSError *error = [NSError errorWithDomain:@"NSURLErrorDomain" code:-1009 userInfo:nil];
        dispatchEvent(L, REQUEST_PRODUCTS_COMPLETE, error, NULL, NULL, NULL);
    }
    else
    {
        dispatchEvent(L, REQUEST_PRODUCTS_COMPLETE, NULL, response.products, response.invalidProductIdentifiers, NULL);
    }
}

- (void)paymentQueue:(SKPaymentQueue*)queue updatedTransactions:(NSArray*)transactions
{
	for (SKPaymentTransaction* transaction in transactions)
	{
		SKPaymentTransactionState state = transaction.transactionState;
		
		if (state == SKPaymentTransactionStatePurchased ||
			state == SKPaymentTransactionStateFailed ||
			state == SKPaymentTransactionStateRestored)
		{
			dispatchEvent(L, TRANSACTION, NULL, NULL, NULL, transaction);
		}
	}
}

- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue *)queue
{
    dispatchEvent(L, RESTORE_TRANSACTIONS_COMPLETE, NULL, NULL, NULL, NULL);    
}

- (void)paymentQueue:(SKPaymentQueue *)queue restoreCompletedTransactionsFailedWithError:(NSError *)error
{
    dispatchEvent(L, RESTORE_TRANSACTIONS_COMPLETE, error, NULL, NULL, NULL);
}

@end

class StoreKit : public GEventDispatcherProxy
{
public:
	StoreKit(lua_State* L) : L(L)
	{
		helper = [[StoreKitHelper alloc] initWithLuaState:L];
		
		[[SKPaymentQueue defaultQueue] addTransactionObserver:helper];		
	}
	
	~StoreKit()
	{
		[[SKPaymentQueue defaultQueue] removeTransactionObserver:helper];
        
        for (std::set<SKRequest*>::iterator iter = s_requests.begin(); iter != s_requests.end(); ++iter)
        {
            SKRequest *request = *iter;
            [request cancel];
            [request release];
        }
        s_requests.clear();

		[helper release];
	}

	BOOL canMakePayments()
	{
		return [SKPaymentQueue canMakePayments];
	}

	void requestProducts(NSSet* productIdentifiers)
	{
		SKProductsRequest* request = [[SKProductsRequest alloc] initWithProductIdentifiers:productIdentifiers];
		request.delegate = helper;
        s_requests.insert(request);
		[request start];
	}

	void purchase(NSString* productIdentifier, int quantity)
	{
		SKMutablePayment *payment = [[[SKMutablePayment alloc] init] autorelease];
		payment.productIdentifier = productIdentifier;
		payment.quantity = quantity;
		[[SKPaymentQueue defaultQueue] addPayment:payment];
	}
	
	void restoreCompletedTransactions()
	{
		[[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
	}
	
	void finishTransaction(SKPaymentTransaction* transaction)
	{
		[[SKPaymentQueue defaultQueue] finishTransaction:transaction];
	}

private:
	lua_State* L;
	StoreKitHelper* helper;
};

static int destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	StoreKit* storekit = static_cast<StoreKit*>(object->proxy());
	
	storekit->unref();
	
	return 0;
}

static StoreKit* getInstance(lua_State* L, int index)
{
	GReferenced* object = static_cast<GReferenced*>(g_getInstance(L, "StoreKit", index));
	StoreKit* storekit = static_cast<StoreKit*>(object->proxy());
	
	return storekit;
}


static int canMakePayments(lua_State* L)
{
	StoreKit* storekit = getInstance(L, 1);

	lua_pushboolean(L, storekit->canMakePayments());
	
	return 1;
}

static int requestProducts(lua_State* L)
{	
	StoreKit* storekit = getInstance(L, 1);
	
    luaL_checktype(L, 2, LUA_TTABLE);
    
	int len = (int)lua_objlen(L, 2);
	
	NSMutableSet* productIdentifiers = [NSMutableSet setWithCapacity:len];
	
	for (int i = 1; i <= len; i++)
    {
		lua_rawgeti(L, 2, i);
        [productIdentifiers addObject:[NSString stringWithUTF8String:luaL_checkstring(L, -1)]];
		lua_pop(L, 1);
    }
	
	storekit->requestProducts(productIdentifiers);
	
	return 0;
}

static int purchase(lua_State* L)
{
	StoreKit* storekit = getInstance(L, 1);
	
	NSString* productIdentifier = [NSString stringWithUTF8String:luaL_checkstring(L, 2)];
    int quantity = (int)luaL_optinteger(L, 3, 1);
	
    storekit->purchase(productIdentifier, quantity);
	
	return 0;
}

static int restoreCompletedTransactions(lua_State* L)
{
	StoreKit* storekit = getInstance(L, 1);
	
	storekit->restoreCompletedTransactions();

	return 0;
}

static int finishTransaction(lua_State* L)
{
	StoreKit* storekit = getInstance(L, 1);
	
	lua_getfield(L, 2, "__userdata");
	luaL_checktype(L, -1, LUA_TLIGHTUSERDATA);
	SKPaymentTransaction* transaction = (SKPaymentTransaction*)lua_touserdata(L, -1);
	lua_pop(L, 1);

	storekit->finishTransaction(transaction);
	
	return 0;
}

static int loader(lua_State* L)
{
	const luaL_Reg functionlist[] = {
		{"canMakePayments", canMakePayments},
		{"requestProducts", requestProducts},
		{"purchase", purchase},
		{"restoreCompletedTransactions", restoreCompletedTransactions},
		{"finishTransaction", finishTransaction},
		{NULL, NULL},
	};
	
	g_createClass(L, "StoreKit", "EventDispatcher", NULL, destruct, functionlist);
	
	lua_getglobal(L, "Event");
	lua_pushstring(L, REQUEST_PRODUCTS_COMPLETE);
	lua_setfield(L, -2, "REQUEST_PRODUCTS_COMPLETE");
	lua_pushstring(L, TRANSACTION);
	lua_setfield(L, -2, "TRANSACTION");
	lua_pushstring(L, RESTORE_TRANSACTIONS_COMPLETE);
	lua_setfield(L, -2, "RESTORE_TRANSACTIONS_COMPLETE");
	lua_pop(L, 1);
	
    
    lua_getglobal(L, "StoreKit");
	lua_pushstring(L, PURCHASED);
	lua_setfield(L, -2, "PURCHASED");
	lua_pushstring(L, RESTORED);
	lua_setfield(L, -2, "RESTORED");
	lua_pushstring(L, FAILED);
	lua_setfield(L, -2, "FAILED");
	lua_pop(L, 1);
    
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of KEY_OBJECTS
	lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
	lua_newtable(L);                  // create a table
	lua_pushliteral(L, "v");
	lua_setfield(L, -2, "__mode");    // set as weak-value table
	lua_pushvalue(L, -1);             // duplicate table
	lua_setmetatable(L, -2);          // set itself as metatable
	lua_rawset(L, LUA_REGISTRYINDEX);	
	
	StoreKit* storekit = new StoreKit(L);
	g_pushInstance(L, "StoreKit", storekit->object());
	
	lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushvalue(L, -2);
	lua_rawseti(L, -2, 1);
	lua_pop(L, 1);	
	
	lua_pushvalue(L, -1);
	lua_setglobal(L, "storekit");
	
	return 1;
}

static void g_initializePlugin(lua_State* L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "storekit");
	
	lua_pop(L, 2);	
}

static void g_deinitializePlugin(lua_State *L)
{
	
}

REGISTER_PLUGIN("Store Kit", "1.0")

