#include <ghttp.h>
#include <map>
#include <stdlib.h>
#include <deque>

struct Connection
{
    g_id id;
    gevent_Callback callback;
    void *udata;
    NSMutableData *data;
	long long expectedContentLength;
    int httpStatusCode;
	NSData *postData;
};

@interface HTTPManager : NSObject
{
    std::map<NSURLConnection*, Connection> map_;
}

@end

@implementation HTTPManager

- (id) init
{
    if (self = [super init])
    {
    }
    return self;
}

- (int)request:(const char*)url :(NSString*)method :(const ghttp_Header *)header :(const void*)data :(size_t)size :(gevent_Callback)callback :(void*)udata
{
    NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
	
	[request setHTTPMethod:method];
    
    [request setValue:@"Gideros" forHTTPHeaderField:@"User-Agent"];
	
    for (; header && header->name; ++header)
		[request setValue:[NSString stringWithUTF8String:header->value] forHTTPHeaderField:[NSString stringWithUTF8String:header->name]];
	
	NSData* postData = nil;
	if (size > 0)
	{
		postData = [[NSData alloc] initWithBytes:data length:size];
		[request setHTTPBody:postData];
	}
	
    NSURLConnection* connection = [NSURLConnection connectionWithRequest:request delegate:self];
    
    Connection connection2;
    connection2.id = g_NextId();
    connection2.callback = callback;
    connection2.udata = udata;
    connection2.data = [[NSMutableData alloc] init];
    connection2.httpStatusCode = -1;
	connection2.postData = postData;
	
    map_[connection] = connection2;
    
    return connection2.id;
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
    if (map_.find(connection) == map_.end())
        return;

    Connection& connection2 = map_[connection];
	
	[connection2.data setLength:0];
	
	connection2.expectedContentLength = response.expectedContentLength;
    connection2.httpStatusCode = [(NSHTTPURLResponse*)response statusCode];
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
    if (map_.find(connection) == map_.end())
        return;

    Connection& connection2 = map_[connection];
    
    [connection2.data appendData:data];
	
    ghttp_ProgressEvent *event = (ghttp_ProgressEvent*)malloc(sizeof(ghttp_ProgressEvent));
    event->bytesLoaded = connection2.data.length;
    event->bytesTotal = connection2.expectedContentLength;
    
    gevent_EnqueueEvent(connection2.id, connection2.callback, GHTTP_PROGRESS_EVENT, event, 1, connection2.udata);
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
    if (map_.find(connection) == map_.end())
        return;
	
    Connection& connection2 = map_[connection];

    ghttp_ResponseEvent *event = (ghttp_ResponseEvent*)malloc(sizeof(ghttp_ResponseEvent) + connection2.data.length);
    event->data = (char*)event + sizeof(ghttp_ResponseEvent);
    memcpy(event->data, connection2.data.bytes, connection2.data.length);
    event->size = connection2.data.length;
    event->httpStatusCode = connection2.httpStatusCode;
	
    gevent_EnqueueEvent(connection2.id, connection2.callback, GHTTP_RESPONSE_EVENT, event, 1, connection2.udata);

	[connection2.data release];
	if (connection2.postData)
		[connection2.postData release];
	map_.erase(connection);
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)err
{
    if (map_.find(connection) == map_.end())
        return;
	
    Connection& connection2 = map_[connection];
		
    ghttp_ErrorEvent *event = (ghttp_ErrorEvent*)malloc(sizeof(ghttp_ErrorEvent));
    
    gevent_EnqueueEvent(connection2.id, connection2.callback, GHTTP_ERROR_EVENT, event, 1, connection2.udata);

	[connection2.data release];
	if (connection2.postData)
		[connection2.postData release];
	map_.erase(connection);
}


-(void) close:(g_id)id
{
    std::map<NSURLConnection*, Connection>::iterator iter = map_.begin(), e = map_.end();
    for (; iter != e; ++iter)
    {
        if (iter->second.id == id)
        {
			[iter->first cancel];
			[iter->second.data release];
			if (iter->second.postData)
				[iter->second.postData release];
            map_.erase(iter);
            break;
        }
    }
    
    gevent_RemoveEventsWithGid(id);
}

-(void) closeAll
{
    while (!map_.empty())
        [self close:map_.begin()->second.id];
}

@end

static HTTPManager* s_manager = nil;

extern "C" {

void ghttp_Init()
{
    s_manager = [[HTTPManager alloc] init];
}

void ghttp_Cleanup()
{
    ghttp_CloseAll();

	[s_manager release];
    s_manager = nil;
}

g_id ghttp_Get(const char *url, const ghttp_Header *header, gevent_Callback callback, void *udata)
{
    return [s_manager request:url:@"GET":header:NULL:0:callback:udata];
}

g_id ghttp_Post(const char *url, const ghttp_Header *header, const void* data, size_t size, gevent_Callback callback, void *udata)
{
	return [s_manager request:url:@"POST":header:data:size:callback:udata];
}

g_id ghttp_Put(const char *url, const ghttp_Header *header, const void* data, size_t size, gevent_Callback callback, void *udata)
{
	return [s_manager request:url:@"PUT":header:data:size:callback:udata];
}

g_id ghttp_Delete(const char *url, const ghttp_Header *header, gevent_Callback callback, void *udata)
{
    return [s_manager request:url:@"DELETE":header:NULL:0:callback:udata];
}

void ghttp_Close(g_id id)
{
	[s_manager close:id];
}

void ghttp_CloseAll()
{
	[s_manager closeAll];
}

}