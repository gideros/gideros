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
#include <string>
#include <map>

void gshare_Init();
bool gshare_Share(std::map<std::string,std::string> values);
void gshare_Cleanup();
    
#endif/* gstoreReview_h */

