//  Created by Nicolas BOUQUET.
//  Copyright � 2017 Gideros Mobile. All rights reserved.
//

#include "sharebinder.h"
#include <StoreKit/StoreKit.h>
#include "gplugin.h"

void gshare_Init()
{
}
void gshare_Cleanup()
{
}

bool gshare_Share(const char *mimeType,const void *data,size_t datasize)
{
    if ([UIDevice currentDevice].systemVersion.floatValue >= 6) {
    	NSObject *obj=NULL;
		NSData *ndata = [NSData dataWithBytes:data length:datasize];
		
    	obj=ndata;
    	if (strstr(mimeType,"image/")==mimeType)
    	{
		 obj= [[UIImage alloc] initWithData:ndata];
    	}
    	else if (!strcmp(mimeType,"text/vcard")) {
    	 obj=[[NSItemProvider alloc] initWithItem:ndata typeIdentifier:kUTTypeVCard];
    	}
    	else if (!strcmp(mimeType,"text/uri-list")) {
    	 obj=[NSURL URLWithString:ndata];
    	}
    	else if (strstr(mimeType,"text/")==mimeType)
    	{
    	 obj=[[NSString alloc] initWithData:ndata encoding:NSUTF8StringEncoding];
    	}
    	
   		NSArray *objectsToShare = @[obj];
 
    	UIActivityViewController *activityVC = [[UIActivityViewController alloc] initWithActivityItems:objectsToShare applicationActivities:nil];
     	[g_getRootViewController() presentViewController:activityVC animated:YES completion:nil];
        return true;
    }
    
    return false;
}
