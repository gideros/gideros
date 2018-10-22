//
//  gstoreReview.h
//  OnlineWordHunt
//
//  Created by Mert Can KURUM on 13/02/17.
//  Copyright © 2017 Gideros Mobile. All rights reserved.
//

#ifndef gmegacool_h_
#define gmegacool_h_

#include <gglobal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GMEGACOOL_EVENT_RECEIVED_SHARE_OPENED	1
#define GMEGACOOL_EVENT_SENT_SHARE_OPENED		2

void gmegacool_Init();
void gmegacool_Destroy();
bool gmegacool_Share(const char *fallbackMedia);
bool gmegacool_StartRecording();
bool gmegacool_SetSharingText(const char *shareText);
void gmegacool_StopRecording();
void gmegacool_Event(int type);
    
#ifdef __cplusplus
}
#endif

#endif/* gmegacool_h */

