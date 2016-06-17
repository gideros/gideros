#include <stdio.h>
#include <string>
#include <map>
#include <ghttp.h>
#include <pthread.h>
#include <curl/curl.h>

struct NetworkReply
{
  g_id id;
  gevent_Callback callback;
  void *udata;
  std::string url;
};

struct MemoryStruct {
  char *memory;
  size_t size;
};

static std::map<g_id, NetworkReply> map_;

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  MemoryStruct *mem = (MemoryStruct *)userp;
 
  mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

void *pull_one_url(void *ptr)
{
  CURL *curl;
  CURLcode res;

  printf("pointer = %p %d\n",ptr, *((g_id*)ptr));
  g_id gid = *((g_id *)ptr);
  printf("pull_one_url gid=%d\n",gid);

  NetworkReply reply2 = map_[gid];

  MemoryStruct chunk;

  chunk.memory = (char*)malloc(1);
  chunk.memory[0]='\0';
  chunk.size = 0;
  
  printf("Processing: %s\n",reply2.url.c_str());

  curl = curl_easy_init();

  curl_easy_setopt(curl, CURLOPT_URL, reply2.url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);                // ignore SSL errors

  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if(res != CURLE_OK)
    fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
 
  ghttp_ResponseEvent *event = (ghttp_ResponseEvent*)malloc(sizeof(ghttp_ResponseEvent) + chunk.size);

  event->data = (char*)event + sizeof(ghttp_ResponseEvent);
  memcpy(event->data, chunk.memory, chunk.size);

  event->size = chunk.size;
  event->httpStatusCode = res;
  event->headers[0].name=NULL;    // no idea what this is for!
  event->headers[0].value=NULL;

  free(chunk.memory);

  printf("enqueue event\n");

  gevent_EnqueueEvent(reply2.id, reply2.callback, GHTTP_RESPONSE_EVENT, event, 1, reply2.udata);

  map_.erase(gid);

  return NULL;
}

extern "C" {

void ghttp_Init()
{
  curl_global_init(CURL_GLOBAL_ALL);
}

void ghttp_Cleanup()
{
  curl_global_cleanup();
}

g_id ghttp_Get(const char* url, const ghttp_Header *header, gevent_Callback callback, void* udata)
{
  pthread_t tid;
  int error;

  static g_id gid = g_NextId();     // must be static

  NetworkReply reply2;
  reply2.id = gid;
  reply2.callback = callback;
  reply2.udata = udata;
  reply2.url = url;

  printf("string=%s\n",reply2.url.c_str());

  map_[gid] = reply2;

  printf("ghttp_Get: %d %p %s\n",gid,&gid,url);

  error = pthread_create(&tid,
			 NULL, /* default attributes please */
			 pull_one_url,
			 (void *)&gid);
  if (0 != error)
    fprintf(stderr, "Couldn't run thread, errno %d\n", error);

  return gid;
}

g_id ghttp_Post(const char* url, const ghttp_Header *header, const void* data, size_t size, gevent_Callback callback, void* udata)
{
//    return s_manager->Post(url, header, data, size, callback, udata);
	return 0;
}

g_id ghttp_Delete(const char* url, const ghttp_Header *header, gevent_Callback callback, void* udata)
{
//    return s_manager->Delete(url, header, callback, udata);
	return 0;
}

g_id ghttp_Put(const char* url, const ghttp_Header *header, const void* data, size_t size, gevent_Callback callback, void* udata)
{
//    return s_manager->Put(url, header, data, size, callback, udata);
	return 0;
}

void ghttp_Close(g_id id)
{
//    s_manager->Close(id);
}

void ghttp_CloseAll()
{
//    s_manager->CloseAll();
}

void ghttp_IgnoreSSLErrors()
{
}

void ghttp_SetProxy(const char *host, int port, const char *user, const char *pass)
{
}

}
