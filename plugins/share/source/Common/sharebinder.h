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
#include <gevent.h>
#include <string>
#include <map>

enum
{
    GFILESHARE_IMPORT_RESULT_EVENT,
    GFILESHARE_EXPORT_RESULT_EVENT,
};

#define STS_OK 				1
#define STS_CANCELED 		0
#define STS_GENERIC_ERR 	-1
#define STS_FILE_NOT_FOUND 	-2
#define STS_BUSY 			-3

#define CAP_SHARE	1
#define CAP_IMPORT	2
#define CAP_EXPORT	4

typedef struct gfileshare_ResultEvent
{
    int status;
    size_t dataSize;
    const char *name;
    const char *mime;
} gfileshare_ResultEvent;


void gshare_Init();
void gshare_Cleanup();
bool gshare_Share(std::map<std::string,std::string> values);
bool gshare_Import(const char *mime, const char *extension);
bool gshare_Export(const char *data,size_t dataSize,const char *mime, const char *filename);
int gshare_Capabilities();
g_id gshare_AddCallback(gevent_Callback callback, void *udata);
void gshare_RemoveCallback(gevent_Callback callback, void *udata);
    
#endif/* gshare_h */

