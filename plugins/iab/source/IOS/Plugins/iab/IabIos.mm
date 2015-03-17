//
//  IabIos.m
//  IAB
//
//  Created by Arturs Sosins on 10/15/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import "IabIos.h"

@implementation IabIos

-(id)init{
    self.iap = @"ios";
    self.prods = nil;
    self.prodKeys = nil;
    [[SKPaymentQueue defaultQueue] addTransactionObserver:self];
    return self;
}

-(void)deinit{
    [[SKPaymentQueue defaultQueue] removeTransactionObserver:self];
    if(self.prods != nil)
    {
        [self.prods release];
        self.prods = nil;
    }
    
    if(self.prodKeys != nil)
    {
        [self.prodKeys release];
        self.prodKeys = nil;
    }
}

-(void)check{
    if ([SKPaymentQueue canMakePayments] == YES)
        giab_onAvailable([self.iap UTF8String]);
    else
        giab_onNotAvailable([self.iap UTF8String]);
}

-(void)setProducts:(NSMutableDictionary*) prods{
    self.prods = [[NSMutableDictionary alloc]initWithDictionary:prods];
    self.prodKeys = [[NSMutableDictionary alloc] init];
    for(NSString *key in self.prods){
        [self.prodKeys setObject:key forKey:[self.prods objectForKey:key]];
    }
}

-(void)setConsumables{
    
}

-(void)request{
    NSMutableSet* productIdentifiers = [NSMutableSet setWithCapacity:[self.prods count]];
	
	for(NSString *key in self.prods){
         [productIdentifiers addObject:[self.prods objectForKey:key]];
    }
    SKProductsRequest* request = [[SKProductsRequest alloc] initWithProductIdentifiers:productIdentifiers];
    request.delegate = self;
    [request start];
}

-(void)purchase:(NSString*)prod{
    SKMutablePayment *payment = [[[SKMutablePayment alloc] init] autorelease];
    payment.productIdentifier = [self.prods objectForKey:prod];
    payment.quantity = 1;
    [[SKPaymentQueue defaultQueue] addPayment:payment];
}

-(void)restore{
    [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
}

-(void)finishTransaction:(SKPaymentTransaction*)transaction
{
    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
}

- (void)request:(SKRequest *)request didFailWithError:(NSError *)error
{
	giab_onProductsError([self.iap UTF8String], [[error localizedDescription] UTF8String]);
}

- (void)productsRequest:(SKProductsRequest*)request didReceiveResponse:(SKProductsResponse*)response
{
    std::vector<Product> prods;
    for(int i = 0; i < [response.products count]; ++i)
    {
        SKProduct* product = [response.products objectAtIndex:i];
        NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
        [numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
        [numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
        [numberFormatter setLocale:product.priceLocale];
        NSString *formattedString = [numberFormatter stringFromNumber:product.price];
        Product gprod = {[[self.prodKeys objectForKey:[product productIdentifier]] UTF8String], [[product localizedTitle] UTF8String], [[product localizedDescription] UTF8String], [formattedString UTF8String]};
        prods.push_back(gprod);
    }
    Product gprod = {"", "", "", ""};
    prods.push_back(gprod);
    giab_onProductsComplete([self.iap UTF8String], &prods[0]);
    [request autorelease];
}

- (void)paymentQueue:(SKPaymentQueue*)queue updatedTransactions:(NSArray*)transactions
{
	for (SKPaymentTransaction* transaction in transactions)
	{
		SKPaymentTransactionState state = transaction.transactionState;
		
		if(state == SKPaymentTransactionStateFailed)
        	{
	            [self finishTransaction:transaction];
	            giab_onPurchaseError([self.iap UTF8String], [transaction.error.localizedDescription UTF8String]);
	        }
		else if(state == SKPaymentTransactionStateRestored || state == SKPaymentTransactionStatePurchased)
		{
		        [self finishTransaction:transaction];
			giab_onPurchaseComplete([self.iap UTF8String], [[self.prodKeys objectForKey:transaction.payment.productIdentifier] UTF8String], [[transaction transactionIdentifier] UTF8String]);
		}
	}
}

- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue *)queue
{
    giab_onRestoreComplete([self.iap UTF8String]);
}

- (void)paymentQueue:(SKPaymentQueue *)queue restoreCompletedTransactionsFailedWithError:(NSError *)error
{
    giab_onRestoreError([self.iap UTF8String], [[error localizedDescription] UTF8String]);
}

@end
