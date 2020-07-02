//
//  gstoreReview.h
//  OnlineWordHunt
//
//  Created by Mert Can KURUM on 13/02/17.
//  Copyright © 2017 Gideros Mobile. All rights reserved.
//

#ifndef gshare_h_
#define gshare_h_

#include <gglobal.h>

#ifdef __cplusplus
extern "C" {
#endif

void gshare_Init();
bool gshare_Share(const char *mimeType,const void *data,size_t datasize);
void gshare_Cleanup();
    
#ifdef __cplusplus
}
#endif

#endif/* gstoreReview_h */

