#include <ghttp.h>
#include <map>
#include <stdlib.h>
#include <deque>

static bool sslErrorsIgnore=false;

struct Connection
{
    g_id id;
    gevent_Callback callback;
    void *udata;
    NSMutableData *data;
	long long expectedContentLength;
    int httpStatusCode;
	NSData *postData;
	NSDictionary *headers;
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
    connection2.headers=[[(NSHTTPURLResponse*)response allHeaderFields] copy];
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
    
    NSInteger hdrCount=[connection2.headers count];
	id hvalue[hdrCount];
	id hname[hdrCount];
    unsigned long hvptr[hdrCount];
    unsigned long hnptr[hdrCount];
	[connection2.headers getObjects:hvalue andKeys:hname];
	NSMutableData *hdrs=[[NSMutableData alloc] init];
	char zero=0;

	for (int i = 0; i < hdrCount; i++) { 
		hnptr[i]=hdrs.length;
		NSData* d=[((NSString *)hname[i]) dataUsingEncoding:NSUTF8StringEncoding];
		[hdrs appendData:d];
		[hdrs appendBytes:&zero length:1];
		hvptr[i]=hdrs.length;
		d=[((NSString *)hvalue[i]) dataUsingEncoding:NSUTF8StringEncoding];
		[hdrs appendData:d];
		[hdrs appendBytes:&zero length:1];
	}
	unsigned long hdrSize=hdrs.length;

    ghttp_ResponseEvent *event = (ghttp_ResponseEvent*)malloc(sizeof(ghttp_ResponseEvent) + sizeof(ghttp_Header)*hdrCount  + connection2.data.length + hdrSize);
    event->data = (char*)event + sizeof(ghttp_ResponseEvent) + sizeof(ghttp_Header)*hdrCount ;
    memcpy(event->data, connection2.data.bytes, connection2.data.length);
	char *hdrData=(char *)(event->data)+connection2.data.length;
    memcpy(hdrData, hdrs.bytes, hdrSize);
    event->size = connection2.data.length;
    event->httpStatusCode = connection2.httpStatusCode;
	for (int i = 0; i < hdrCount; i++) { 
		event->headers[i].name=hdrData+hnptr[i];
		event->headers[i].value=hdrData+hvptr[i];
	}
	event->headers[hdrCount].name=NULL;
	event->headers[hdrCount].value=NULL;
    [hdrs release];
	
    gevent_EnqueueEvent(connection2.id, connection2.callback, GHTTP_RESPONSE_EVENT, event, 1, connection2.udata);

	
	[connection2.headers release];
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

- (BOOL)connection:(NSURLConnection *)connection canAuthenticateAgainstProtectionSpace:(NSURLProtectionSpace *)protectionSpace {
  return [protectionSpace.authenticationMethod isEqualToString:NSURLAuthenticationMethodServerTrust];
}

- (void)connection:(NSURLConnection *)connection didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge {
  if (sslErrorsIgnore)
  	[challenge.sender useCredential:[NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust] forAuthenticationChallenge:challenge];
  [challenge.sender continueWithoutCredentialForAuthenticationChallenge:challenge];
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
void ghttp_IgnoreSSLErrors()
{
	sslErrorsIgnore=true;
}

void ghttp_SetProxy(const char *host, int port, const char *user, const char *pass)
{
}

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