#include <ghttp.h>

using namespace Windows::Web::Http;
using namespace Windows::Foundation;

extern "C" {

void ghttp_Init()
{
//    s_manager = new HTTPManager();
}

void ghttp_Cleanup()
{
//    delete s_manager;
//    s_manager = NULL;
}

g_id ghttp_Get(const char* url, const ghttp_Header *header, gevent_Callback callback, void* udata)
{
//    return s_manager->Get(url, header, callback, udata);

//	HttpClient ^client = ref new HttpClient();
//	Uri ^uri = ref new Uri("http://example.com/datalist.aspx");

//	auto obj = client->GetAsync(uri);
//	obj->GetResults();

	return 0;
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
