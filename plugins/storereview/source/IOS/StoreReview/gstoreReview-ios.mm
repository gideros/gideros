//
//  gstoreReview-ios.mm
//  OnlineWordHunt
//
//  Created by Mert Can KURUM on 13/02/17.
//  Copyright © 2017 Gideros Mobile. All rights reserved.
//

#include "gstoreReview.h"
//#import "GAI.h"
#include <StoreKit/StoreKit.h>

bool gstorereview_requestReview()
{
    if ([UIDevice currentDevice].systemVersion.floatValue >= 10.3) {
        [SKStoreReviewController requestReview];
        return true;
    }
    
    return false;
}
