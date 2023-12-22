//  Created by Nicolas BOUQUET.
//  Copyright � 2017 Gideros Mobile. All rights reserved.
//

#include "sharebinder.h"
#include <StoreKit/StoreKit.h>
#include "gplugin.h"
#include <stdlib.h>
#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

static gevent_CallbackList callbackList_;
static void callback_s(int type, void *event, void *udata)
{
  callbackList_.dispatchEvent(type, event);
}


static g_id gid = g_NextId();

void onImportResult(int status, const char *sname, const char *smime, const char *sdata, size_t sdataSize)
{
	gfileshare_ResultEvent *event = (gfileshare_ResultEvent*)gevent_CreateEventStruct2(sizeof(gfileshare_ResultEvent)+sdataSize,
			offsetof(gfileshare_ResultEvent,name),sname,
			offsetof(gfileshare_ResultEvent,mime),smime);
	memcpy((char *)(event+1),sdata,sdataSize);
	event->dataSize=sdataSize;
	event->status=status;

	gevent_EnqueueEvent(gid, callback_s, GFILESHARE_IMPORT_RESULT_EVENT, event, 1, NULL);
}

void onExportResult(int status)
{
	gfileshare_ResultEvent *event = (gfileshare_ResultEvent*)gevent_CreateEventStruct2(sizeof(gfileshare_ResultEvent),
			offsetof(gfileshare_ResultEvent,name),NULL,
			offsetof(gfileshare_ResultEvent,mime),NULL);
	event->status=status;
	event->dataSize=0;

	gevent_EnqueueEvent(gid, callback_s, GFILESHARE_EXPORT_RESULT_EVENT, event, 1, NULL);
}

@interface FileShareManager : NSObject<UIDocumentPickerDelegate>
{
    int mode;
    NSURL *exportUrl;
}

- (id) init;

- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray <NSURL *>*)urls;

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller;

@end

@implementation FileShareManager

- (id)init
{
    if (self = [super init])
    {
        mode=0;
    }
    
    return self;
}

- (void)dealloc
{
}

- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray <NSURL *>*)urls
{
    if (@available(iOS 14,*)) {
        if (mode==1) {
            NSURL *url=[urls objectAtIndex:0];
            NSData *data=NULL;;
            if ([url startAccessingSecurityScopedResource]) {
                NSError *err=NULL;
                data=[NSData dataWithContentsOfURL:url options:NSDataReadingUncached error:&err];
            }
            if (data) {
                const char *mime=NULL;
                NSString *ext=[url pathExtension];
                UTType *text=[UTType typeWithFilenameExtension:ext];
                if (text) {
                    NSString *tmime=[text preferredMIMEType];
                    mime=tmime?[tmime UTF8String]:NULL;
                }
                
                onImportResult(STS_OK,mime,[[url lastPathComponent] UTF8String],(const char *)[data bytes],[data length]);
            }
            else
                onImportResult(STS_GENERIC_ERR,NULL,NULL,NULL,0);
        }
        if (mode==-1) {
            [[NSFileManager defaultManager] removeItemAtURL:exportUrl error:NULL];
            exportUrl=nil;
            onExportResult(STS_OK);
        }
    }
    mode=0;
}

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller
{
    if (mode==1)
        onImportResult(STS_CANCELED,NULL,NULL,NULL,0);
    if (mode==-1) {
        [[NSFileManager defaultManager] removeItemAtURL:exportUrl error:NULL];
        exportUrl=nil;
        onExportResult(STS_CANCELED);
    }
    mode=0;
}

- (bool) fileImport:(NSString *) mime extension:(NSString *) extension
{
    if (@available(iOS 14,*)) {
        if (mode) {
            onImportResult(STS_BUSY,NULL,NULL,NULL,0);
        }
        else
        {
            NSMutableArray<UTType *> *types=[[NSMutableArray alloc] init];
            if (mime) {
                UTType *tmime=[UTType typeWithMIMEType:mime];
                if (tmime)
                    [types addObject:tmime];
            }
            if (extension) {
                UTType *text=[UTType typeWithFilenameExtension:extension];
                if (text)
                    [types addObject:text];
            }
            UIDocumentPickerViewController *picker=[[UIDocumentPickerViewController alloc] initForOpeningContentTypes:types];
            picker.delegate=self;
            mode=1;
            [g_getRootViewController() presentViewController:picker animated:YES completion:nil];
        }
        return true;
    }
    else
        return false;
}

- (bool) fileExport:(NSData *) data mime:(NSString *) mime filename:(NSString *) filename
{
    if (@available(iOS 14,*)) {
        if (mode) {
            onExportResult(STS_BUSY);
        }
        else
        {
            NSString *tmpdir=[NSTemporaryDirectory()
                             stringByAppendingPathComponent:@"_GID_Plugin_Share_"];
            NSFileManager *fm=[NSFileManager defaultManager];
            if (![fm fileExistsAtPath:tmpdir])
                [fm createDirectoryAtPath:tmpdir withIntermediateDirectories:TRUE attributes:nil error:NULL];
            NSString *tmpfile = [tmpdir stringByAppendingPathComponent:filename];
            if ([fm createFileAtPath:tmpfile contents:data attributes:nil]) {
                exportUrl=[NSURL fileURLWithPath:tmpfile];
                UIDocumentPickerViewController *picker=[[UIDocumentPickerViewController alloc] initForExportingURLs:@[ exportUrl ]];
                picker.delegate=self;
                mode=-1;
                [g_getRootViewController() presentViewController:picker animated:YES completion:nil];
            }
            else
                onExportResult(STS_FILE_NOT_FOUND);
        }
        return true;
    }
    else
        return false;

}

@end

FileShareManager *fsm;

void gshare_Init()
{
    fsm=[[FileShareManager alloc] init];
}

void gshare_Cleanup()
{
}

bool gshare_Share(std::map<std::string,std::string> values)
{
    if ([UIDevice currentDevice].systemVersion.floatValue >= 6) {
    	NSMutableArray *objectsToShare = [[NSMutableArray alloc] init];
    	
    	std::map<std::string, std::string>::const_iterator citr = values.begin();
		for( ; citr != values.end(); ++citr) {
    	
	    	NSObject *obj=NULL;
	    	const char *mimeType=citr->first.c_str();
	    	const char *data=citr->second.c_str();
			size_t datasize = citr->second.size();
			NSData *ndata = [NSData dataWithBytes:data length:datasize];
			
	    	obj=ndata;
	    	if (strstr(mimeType,"image/")==mimeType)
	    	{
			 obj= [[UIImage alloc] initWithData:ndata];
	    	}
	    	else if (!strcmp(mimeType,"text/vcard")) {
	    	 obj=[[NSItemProvider alloc] initWithItem:ndata typeIdentifier:@"public.vcard"];
	    	}
	    	else if (!strcmp(mimeType,"text/uri-list")) {
	    	 obj=[NSURL URLWithString:[[NSString alloc] initWithData:ndata encoding:NSUTF8StringEncoding]];
	    	}
	    	else if (strstr(mimeType,"text/")==mimeType)
	    	{
	    	 obj=[[NSString alloc] initWithData:ndata encoding:NSUTF8StringEncoding];
	    	}
	    	
			[objectsToShare addObject:obj];
		}
 
    	UIActivityViewController *activityVC = [[UIActivityViewController alloc] initWithActivityItems:objectsToShare applicationActivities:nil];
     	[g_getRootViewController() presentViewController:activityVC animated:YES completion:nil];
        return true;
    }
    
    return false;
}

bool gfileshare_Import(const char *mime, const char *extension)
{
	return [fsm fileImport:mime?[NSString stringWithCString:mime encoding:NSUTF8StringEncoding]:nil
			extension:extension?[NSString stringWithCString:extension encoding:NSUTF8StringEncoding]:nil];
}

bool gfileshare_Export(const char *data,size_t dataSize,const char *mime, const char *filename)
{
	return [fsm fileExport:[NSData dataWithBytes:data length:dataSize]
			mime:[NSString stringWithCString:mime encoding:NSUTF8StringEncoding]
			filename:[NSString stringWithCString:filename encoding:NSUTF8StringEncoding]];
}


g_id gfileshare_AddCallback(gevent_Callback callback, void *udata)
{
    return callbackList_.addCallback(callback, udata);
}

void gfileshare_RemoveCallback(gevent_Callback callback, void *udata)
{
    callbackList_.removeCallback(callback, udata);
}

int gshare_Capabilities()
{
	return CAP_SHARE|CAP_IMPORT|CAP_EXPORT;
}

