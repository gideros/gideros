#include <stdio.h>
#include <string>
#include <map>
#include <ghttp.h>
#include <pthread.h>
#include <curl/curl.h>

static bool sslErrorsIgnore=false;
static std::string colon=": ";

struct NetworkReply
{
  g_id gid;
  gevent_Callback callback;
  void *udata;
  std::string url;
  pthread_t tid;
  void *data;
  size_t size;
  std::vector<std::string> header;
};

struct MemoryStruct {
  char *memory;
  size_t size;
};

static pthread_mutex_t mutexget, mutexput, mutexpost;

static std::map<g_id, NetworkReply> map_;

// ######################################################################

int progress_callback(void *clientp,   double dltotal,   double dlnow,   double ultotal,   double ulnow)
{
  NetworkReply *reply2 = (NetworkReply*)clientp;
  
  ghttp_ProgressEvent* event = (ghttp_ProgressEvent*)malloc(sizeof(ghttp_ProgressEvent));
  event->bytesLoaded = dlnow > ulnow ? dlnow : ulnow;
  event->bytesTotal = dltotal > ultotal ? dltotal : ultotal;
  
  gevent_EnqueueEvent(reply2->gid, reply2->callback, GHTTP_PROGRESS_EVENT, event, 1, reply2->udata);
  
  return 0;
}

//######################################################################

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

//######################################################################

static size_t
ReadMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  MemoryStruct *mem = (MemoryStruct *)userp;

  size_t copysize;

  if (realsize > mem->size)
    copysize=mem->size;
  else
    copysize=realsize;

  memcpy(contents,mem->memory,copysize);
  mem->memory += copysize;
  mem->size -= copysize;

  //  printf("ReadMemoryCallback %d\n",copysize);

  return copysize;
}

// ######################################################################

static void *post_one(void *ptr)        // thread
{
  CURL *curl;
  CURLcode res;

  NetworkReply *reply2 = (NetworkReply*)ptr;

  MemoryStruct chunk;  // for the return message if any

  chunk.memory = (char*)malloc(1);
  chunk.memory[0]='\0';
  chunk.size = 0;

  curl = curl_easy_init();

  struct curl_slist *headers=NULL;

  for (int i=0; i<reply2->header.size(); i++){
    headers = curl_slist_append(headers, reply2->header[i].c_str());
    printf("header %p %s\n",headers,reply2->header[i].c_str());
  }

  curl_easy_setopt(curl, CURLOPT_URL, reply2->url.c_str());
  //  curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

  /* post binary data */
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reply2->data);
 
  /* set the size of the postfields data */
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, reply2->size);
  
  /* pass our list of custom made headers */
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  if (sslErrorsIgnore)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);                // ignore SSL errors
 
  res=curl_easy_perform(curl); /* post away! */

  curl_easy_cleanup(curl);

  if(res != CURLE_OK){
    ghttp_ErrorEvent *event = (ghttp_ErrorEvent*)malloc(sizeof(ghttp_ErrorEvent));
    gevent_EnqueueEvent(reply2->gid, reply2->callback, GHTTP_ERROR_EVENT, event, 1, reply2->udata);
    fprintf(stderr, "curl_easy_perform() failed in post_one: %s\n",curl_easy_strerror(res));
  }
  else {

    ghttp_ResponseEvent *event = (ghttp_ResponseEvent*)malloc(sizeof(ghttp_ResponseEvent) + chunk.size);

    event->data = (char*)event + sizeof(ghttp_ResponseEvent);
    memcpy(event->data, chunk.memory, chunk.size);

    event->size = chunk.size;
    event->httpStatusCode = res;
    event->headers[0].name=NULL;    // no idea what this is for!
    event->headers[0].value=NULL;

    gevent_EnqueueEvent(reply2->gid, reply2->callback, GHTTP_RESPONSE_EVENT, event, 1, reply2->udata);
  }

  free(chunk.memory);
  curl_slist_free_all(headers); /* free the header list */

  pthread_mutex_lock (&mutexpost);
  map_.erase(reply2->gid);
  pthread_mutex_unlock (&mutexpost);

  free(reply2->data);

  return NULL;
}

//######################################################################

static void *put_one_url(void *ptr)     // thread
{
  CURL *curl;
  CURLcode res;

  NetworkReply *reply2 = (NetworkReply*)ptr;

  struct curl_slist *headers=NULL;

  for (int i=0; i<reply2->header.size(); i++){
    headers = curl_slist_append(headers, reply2->header[i].c_str());
    printf("header %p %s\n",headers,reply2->header[i].c_str());
  }

  MemoryStruct chunk;

  chunk.memory=(char*)reply2->data;       // start of the remaining data
  chunk.size=reply2->size;         // size of the remaining data

  MemoryStruct chunkRet;  // for the return message if any

  chunkRet.memory = (char*)malloc(1);
  chunkRet.memory[0]='\0';
  chunkRet.size = 0;

  printf("put_one_url: %s\n",reply2->url.c_str());

  curl=curl_easy_init();

  /* pass our list of custom made headers */
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunkRet);

  curl_easy_setopt(curl, CURLOPT_URL, reply2->url.c_str());
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_READDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

  if (sslErrorsIgnore)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);                // ignore SSL errors

  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if(res != CURLE_OK){
    ghttp_ErrorEvent *event = (ghttp_ErrorEvent*)malloc(sizeof(ghttp_ErrorEvent));
    gevent_EnqueueEvent(reply2->gid, reply2->callback, GHTTP_ERROR_EVENT, event, 1, reply2->udata);
    fprintf(stderr, "curl_easy_perform() failed in put_one: %s\n",curl_easy_strerror(res));
  }
  else {

    ghttp_ResponseEvent *event = (ghttp_ResponseEvent*)malloc(sizeof(ghttp_ResponseEvent) + chunkRet.size);

    event->data = (char*)event + sizeof(ghttp_ResponseEvent);
    memcpy(event->data, chunkRet.memory, chunkRet.size);

    event->size = chunkRet.size;
    event->httpStatusCode = res;
    event->headers[0].name=NULL;    // no idea what this is for!
    event->headers[0].value=NULL;
    
    gevent_EnqueueEvent(reply2->gid, reply2->callback, GHTTP_RESPONSE_EVENT, event, 1, reply2->udata);
  }

  free(chunkRet.memory);
  curl_slist_free_all(headers); /* free the header list */

  pthread_mutex_lock (&mutexput);
  map_.erase(reply2->gid);
  pthread_mutex_unlock (&mutexput);

  free(reply2->data);

  return NULL;
}

//######################################################################

static void *get_one_url(void *ptr)          // thread
{
  CURL *curl;
  CURLcode res;

  NetworkReply *reply2 = (NetworkReply*)ptr;

  struct curl_slist *headers=NULL;

  for (int i=0; i<reply2->header.size(); i++){
    headers = curl_slist_append(headers, reply2->header[i].c_str());
    printf("header %p %s\n",headers,reply2->header[i].c_str());
  }

  MemoryStruct chunk;

  chunk.memory = (char*)malloc(1);
  chunk.memory[0]='\0';
  chunk.size = 0;
  
  printf("Processing: %s\n",reply2->url.c_str());

  curl = curl_easy_init();

  /* pass our list of custom made headers */
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  curl_easy_setopt(curl, CURLOPT_URL, reply2->url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

  curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
  curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, ptr);

  if (sslErrorsIgnore)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);                // ignore SSL errors

  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if(res != CURLE_OK){
    ghttp_ErrorEvent *event = (ghttp_ErrorEvent*)malloc(sizeof(ghttp_ErrorEvent));
    gevent_EnqueueEvent(reply2->gid, reply2->callback, GHTTP_ERROR_EVENT, event, 1, reply2->udata);
    fprintf(stderr, "curl_easy_perform() failed in get_one: %s\n",curl_easy_strerror(res));
  }
  else {
 
    ghttp_ResponseEvent *event = (ghttp_ResponseEvent*)malloc(sizeof(ghttp_ResponseEvent) + chunk.size);
    
    event->data = (char*)event + sizeof(ghttp_ResponseEvent);
    memcpy(event->data, chunk.memory, chunk.size);
    
    event->size = chunk.size;
    event->httpStatusCode = res;
    event->headers[0].name=NULL;    // no idea what this is for!
    event->headers[0].value=NULL;
    
    gevent_EnqueueEvent(reply2->gid, reply2->callback, GHTTP_RESPONSE_EVENT, event, 1, reply2->udata);
  }

  free(chunk.memory);
  curl_slist_free_all(headers); /* free the header list */

  pthread_mutex_lock (&mutexget);
  map_.erase(reply2->gid);
  pthread_mutex_unlock (&mutexget);

  return NULL;
}

//######################################################################

extern "C" {

void ghttp_Init()
{
  pthread_mutex_init(&mutexget, NULL);
  pthread_mutex_init(&mutexput, NULL);
  pthread_mutex_init(&mutexpost, NULL);
  curl_global_init(CURL_GLOBAL_ALL);
}

void ghttp_Cleanup()
{
  curl_global_cleanup();
  pthread_mutex_destroy(&mutexget);
  pthread_mutex_destroy(&mutexput);
  pthread_mutex_destroy(&mutexpost);
}

g_id ghttp_Get(const char* url, const ghttp_Header *header, gevent_Callback callback, void* udata)
{
  pthread_t tid;
  int error;
  std::string str;

  g_id gid = g_NextId();     // must be static

  NetworkReply reply2;
  reply2.gid = gid;
  reply2.callback = callback;
  reply2.udata = udata;
  reply2.url = url;
  reply2.data = NULL;
  reply2.size = 0;

  if (header)
    for (; header->name; ++header){
      str=header->name + colon + header->value;
      reply2.header.push_back(str);
    } 


  map_[gid] = reply2;

  printf("ghttp_Get: %d %p %s\n",gid,&gid,url);

  error = pthread_create(&tid,
			 NULL, /* default attributes please */
			 get_one_url,
			 (void *)&map_[gid]);
  if (0 != error)
    fprintf(stderr, "Couldn't run thread, errno %d\n", error);

  map_[gid].tid=tid;

  return gid;
}

g_id ghttp_Post(const char* url, const ghttp_Header *header, const void* data, size_t size, gevent_Callback callback, void* udata)
{

  pthread_t tid;
  int error;
  std::string str;

  g_id gid = g_NextId();

  NetworkReply reply2;
  reply2.gid = gid;
  reply2.callback = callback;
  reply2.udata = udata;
  reply2.url = url;
  reply2.data = malloc(size);
  reply2.size = size;

  memcpy(reply2.data,data,size);

  if (header)
    for (; header->name; ++header){
      str=header->name + colon + header->value;
      reply2.header.push_back(str);
    } 

  map_[gid] = reply2;

  printf("ghttp_Post: %d %p %s\n",gid,&gid,url);

  error = pthread_create(&tid,
			 NULL, /* default attributes please */
			 post_one,
			 (void *)&map_[gid]);
  if (0 != error)
    fprintf(stderr, "Couldn't run thread, errno %d\n", error);

  map_[gid].tid=tid;

  return gid;
}

g_id ghttp_Delete(const char* url, const ghttp_Header *header, gevent_Callback callback, void* udata)
{

  CURL *curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
  curl_easy_setopt(curl, CURLOPT_URL, url);

  struct curl_slist *headers = NULL;

  if (header)
    for (; header->name; ++header){
      std::string str=header->name + colon + header->value;
      headers = curl_slist_append(headers, str.c_str());
    } 

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  CURLcode ret = curl_easy_perform(curl);
  
  curl_easy_cleanup(curl);
  
  if(ret != CURLE_OK)
    fprintf(stderr, "curl_easy_perform() failed in Delete: %s\n",curl_easy_strerror(ret));
  
  return 0;
}

g_id ghttp_Put(const char* url, const ghttp_Header *header, const void* data, size_t size, gevent_Callback callback, void* udata)
{
  pthread_t tid;
  int error;
  std::string str;

  g_id gid = g_NextId();

  printf("ghttp_Put: size = %d\n",size);

  NetworkReply reply2;
  reply2.gid = gid;
  reply2.callback = callback;
  reply2.udata = udata;
  reply2.url = url;
  reply2.data = malloc(size);
  reply2.size = size;

  memcpy(reply2.data,data,size);

  if (header)
    for (; header->name; ++header){
      str=header->name + colon + header->value;
      reply2.header.push_back(str);
    } 


  map_[gid] = reply2;

  printf("ghttp_Put: %d %p %s\n",gid,&gid,url);

  error = pthread_create(&tid,
			 NULL, /* default attributes please */
			 put_one_url,
			 (void *)&map_[gid]);
  if (0 != error)
    fprintf(stderr, "Couldn't run thread, errno %d\n", error);

  map_[gid].tid=tid;

  return gid;
}

void ghttp_Close(g_id gid)
{
  pthread_cancel(map_[gid].tid);
  map_.erase(gid);
}

void ghttp_CloseAll()
{
  while (!map_.empty())
    ghttp_Close(map_.begin()->second.gid);
}

void ghttp_IgnoreSSLErrors()
{
  sslErrorsIgnore=true;
}

void ghttp_SetProxy(const char *host, int port, const char *user, const char *pass)
{
}

}
