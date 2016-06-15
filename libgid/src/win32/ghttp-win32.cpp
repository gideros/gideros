#include <stdio.h>
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
 
  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
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

static void *pull_one_url(void *ptr)
{
  CURL *curl;
  CURLcode res;

  int gid = *((int *)ptr);
  NetworkReply reply2 = map_[gid];

  MemoryStruct chunk;

  chunk.memory = malloc(1);
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

  free(chunk.memory);

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

  g_id gid = g_NextId();

  NetworkReply reply2;
  reply2.id = gid;
  reply2.callback = callback;
  reply2.udata = udata;
  reply2.url = url;

  map_[gid] = reply2;

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

}
