#include <ghttp.h>
#include "collection.h"
#include <ppltasks.h>
#include <map>
#include <mutex>

using namespace concurrency;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Headers;
using namespace Windows::Web::Http::Filters;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Streams;
using namespace Windows::Security::Cryptography::Certificates;
using namespace Platform;

std::wstring utf8_ws(const char *str);
std::string utf8_us(const wchar_t *str);


static HttpClient^ httpClient;
static cancellation_token_source cancellationTokenSource;
static bool sslErrorsIgnore = false;
static std::map<g_id, std::vector<unsigned char> > map;
static std::mutex mapm;
//static std::vector<unsigned char> dataRead;
task<IBuffer^> readData(IInputStream^ stream, g_id id, bool streaming, gevent_Callback callback, void* udata)
{
	// Do an asynchronous read. We need to use use_current() with the continuations since the tasks are completed on
	// background threads and we need to run on the UI thread to update the UI.
	Buffer^ readBuffer = ref new Buffer(1<<17);
	return create_task(
		stream->ReadAsync(readBuffer, readBuffer->Capacity, InputStreamOptions::Partial),
		cancellationTokenSource.get_token()).then([=](task<IBuffer^> readTask)
	{
		Buffer^ buffer = (Buffer ^)(readTask.get());
		//OutputField->Text += "Bytes read from stream: " + buffer->Length + "\n";
		Array<unsigned char>^ arr = ref new Array<unsigned char>(buffer->Length);
		DataReader::FromBuffer(buffer)->ReadBytes(arr);
		unsigned char* first = arr->Data;
		unsigned char* end = first + arr->Length;

		if (streaming)
		{
			ghttp_ProgressEvent* event = (ghttp_ProgressEvent*)malloc(sizeof(ghttp_ProgressEvent)+arr->Length);
			event->bytesLoaded = 0; //We could count them if really needed
			event->bytesTotal = 0;
			event->chunk = event+1;
			event->chunkSize = arr->Length;
			memcpy(event->chunk, first, arr->Length);

			gevent_EnqueueEvent(id, callback, GHTTP_PROGRESS_EVENT, event, 1, udata);
		}
		else {
			mapm.lock();
			map[id].insert(map[id].end(), first, end);
			mapm.unlock();
		}

		// Continue reading until the response is complete.  When done, return previousTask that is complete.
		bool more = buffer->Length;
		delete buffer;
		return more ? readData(stream, id, streaming, callback, udata) : readTask;
	});
}


void handleException(Exception^ ex, g_id id, gevent_Callback callback, void* udata)
{
	ghttp_ErrorEvent *event = (ghttp_ErrorEvent*)malloc(sizeof(ghttp_ErrorEvent));
	//std::wstring werror(ex->Message->Begin());
	//std::string strerror(werror.begin(), werror.end());
	//strerror.c_str();
	gevent_EnqueueEvent(id, callback, GHTTP_ERROR_EVENT, event, 1, udata);
	mapm.lock();
	map.erase(id);
	mapm.unlock();
}

void handleProcess(IAsyncOperationWithProgress<HttpResponseMessage^, HttpProgress>^ operation, g_id id, gevent_Callback callback, void* udata, bool streaming){
	if (streaming) return;
	operation->Progress = ref new AsyncOperationProgressHandler<HttpResponseMessage^, HttpProgress>([=](
		IAsyncOperationWithProgress<HttpResponseMessage^, HttpProgress>^ asyncInfo,
		HttpProgress progress)
	{
		if (progress.Stage == HttpProgressStage::ReceivingContent)
		{
			unsigned long long totalBytesToReceive = 0;
			if (progress.TotalBytesToReceive != nullptr)
				totalBytesToReceive = progress.TotalBytesToReceive->Value;

			unsigned long long requestProgress = 0;
			if (totalBytesToReceive > 0)
			{
				requestProgress = progress.BytesReceived;
				ghttp_ProgressEvent* event = (ghttp_ProgressEvent*)malloc(sizeof(ghttp_ProgressEvent));
				event->bytesLoaded = requestProgress;
				event->bytesTotal = totalBytesToReceive;
				event->chunk = NULL;
				event->chunkSize = 0;

				gevent_EnqueueEvent(id, callback, GHTTP_PROGRESS_EVENT, event, 1, udata);
			}
		}
		else
		{
			return;
		}
	});
}

void responseEvent(HttpResponseMessage^ response, g_id id, gevent_Callback callback, void* udata, bool header) {
	IIterable<IKeyValuePair<String^, String^>^>^ headers = response->Headers;
	int hdrCount = response->Headers->Size;
	int hdrSize = 0;
	for each(IKeyValuePair<String^, String^>^ pair in headers)
	{
		hdrSize += pair->Key->Length();
		hdrSize += pair->Value->Length();
		hdrSize += 2;
	}

	IIterable<IKeyValuePair<String^, String^>^>^ headers2 = response->Content->Headers;
	hdrCount += response->Content->Headers->Size;
	for each(IKeyValuePair<String^, String^>^ pair in headers2)
	{
		hdrSize += pair->Key->Length();
		hdrSize += pair->Value->Length();
		hdrSize += 2;
	}
	ghttp_ResponseEvent *event = (ghttp_ResponseEvent*)malloc(sizeof(ghttp_ResponseEvent) + sizeof(ghttp_Header)*hdrCount + (header?0:map[id].size()) + hdrSize);

	event->data = (char*)event + sizeof(ghttp_ResponseEvent) + sizeof(ghttp_Header)*hdrCount;
	if (header)
		event->size = 0;
	else {
		memcpy(event->data, map[id].data(), map[id].size());
		event->size = map[id].size();
	}

	if (response->IsSuccessStatusCode)
		event->httpStatusCode = (int)response->StatusCode;
	else
		event->httpStatusCode = -1;

	int hdrn = 0;
	char *hdrData = (char *)(event->data) + map[id].size();
	for each(IKeyValuePair<String^, String^>^ pair in headers)
	{
		int ds = pair->Key->Length();
		std::wstring wkey(pair->Key->Begin());
		std::string strkey = utf8_us(wkey.c_str());
		memcpy(hdrData, strkey.c_str(), ds);
		event->headers[hdrn].name = hdrData;
		hdrData += ds;
		*(hdrData++) = 0;
		ds = pair->Value->Length();
		std::wstring wval(pair->Value->Begin());
		std::string strval = utf8_us(wval.c_str());
		memcpy(hdrData, strval.c_str(), ds);
		event->headers[hdrn].value = hdrData;
		hdrData += ds;
		*(hdrData++) = 0;
		hdrn++;
	}
	for each(IKeyValuePair<String^, String^>^ pair in headers2)
	{
		int ds = pair->Key->Length();
		std::wstring wkey(pair->Key->Begin());
		std::string strkey = utf8_us(wkey.c_str());
		memcpy(hdrData, strkey.c_str(), ds);
		event->headers[hdrn].name = hdrData;
		hdrData += ds;
		*(hdrData++) = 0;
		ds = pair->Value->Length();
		std::wstring wval(pair->Value->Begin());
		std::string strval = utf8_us(wval.c_str());
		memcpy(hdrData, strval.c_str(), ds);
		event->headers[hdrn].value = hdrData;
		hdrData += ds;
		*(hdrData++) = 0;
		hdrn++;
	}
	event->headers[hdrn].name = NULL;
	event->headers[hdrn].value = NULL;

	gevent_EnqueueEvent(id, callback, header?GHTTP_HEADER_EVENT:GHTTP_RESPONSE_EVENT, event, 1, udata);
}

void handleTask(HttpResponseMessage^ response, g_id id, boolean streaming, gevent_Callback callback, void* udata){
	mapm.lock();
	map[id] = std::vector<unsigned char>();
	mapm.unlock();
	create_task(response->Content->ReadAsInputStreamAsync(), cancellationTokenSource.get_token()).then([=](task<IInputStream^> previousTask)
	{
		if (streaming) {
			responseEvent(response, id, callback, udata, true);
		}

		IInputStream^ stream = previousTask.get();
		return readData(stream, id, streaming, callback, udata);
	}).then([=](task<IBuffer^> previousTask)
	{
		try
		{
			// Check if any previous task threw an exception.
			previousTask.get();

			responseEvent(response, id, callback, udata, false);

			//progress finished
			ghttp_ProgressEvent* eventp = (ghttp_ProgressEvent*)malloc(sizeof(ghttp_ProgressEvent));
			eventp->bytesLoaded = map[id].size();
			eventp->bytesTotal = map[id].size();

			gevent_EnqueueEvent(id, callback, GHTTP_PROGRESS_EVENT, eventp, 1, udata);

			mapm.lock();
			map.erase(id);
			mapm.unlock();
		}
		catch (const task_canceled&)
		{
			ghttp_ErrorEvent *event = (ghttp_ErrorEvent*)malloc(sizeof(ghttp_ErrorEvent));
			gevent_EnqueueEvent(id, callback, GHTTP_ERROR_EVENT, event, 1, udata);
			mapm.lock();
			map.erase(id);
			mapm.unlock();
		}
		catch (Exception^ ex)
		{
			handleException(ex, id, callback, udata);
		}
	});
}

void add_headers(const ghttp_Header *header, HttpRequestMessage^ request){
	HttpRequestHeaderCollection^ headers = request->Headers;
	headers->UserAgent->ParseAdd("Gideros");
	if (header){
		for (; header->name; ++header){
			if (_strnicmp(header->name, "content-", 8)) {
				std::wstring wstrkey = utf8_ws(header->name);
				std::wstring wstrval = utf8_ws(header->value);
				headers->Insert(ref new String(wstrkey.c_str()), ref new String(wstrval.c_str()));
			}
		}
	}
}

void add_post_headers(const ghttp_Header *header, HttpBufferContent^ content){
	if (header){
		for (; header->name; ++header){
			if (!_stricmp(header->name, "content-type")) {
				std::wstring wstrkey = utf8_ws(header->name);
				std::wstring wstrval = utf8_ws(header->value);
				content->Headers->ContentType = ref new HttpMediaTypeHeaderValue(ref new String(wstrval.c_str()));
			}
		}
	}
}

extern "C" {

void ghttp_Init()
{
	HttpBaseProtocolFilter^ filter = ref new HttpBaseProtocolFilter();
	filter->AllowAutoRedirect = true;
	httpClient = ref new HttpClient(filter);
}

void ghttp_Cleanup()
{
	
}

g_id ghttp_Get(const char* url, const ghttp_Header *header, int streaming, gevent_Callback callback, void* udata)
{
	std::wstring wstrurl=utf8_ws(url);
	Uri^ uri = ref new Uri(ref new String(wstrurl.c_str()));
	g_id id = g_NextId();

	HttpRequestMessage^ request = ref new HttpRequestMessage(HttpMethod::Get,uri);
	add_headers(header,request);
	
	IAsyncOperationWithProgress<HttpResponseMessage^, HttpProgress>^ operation = httpClient->SendRequestAsync(request, HttpCompletionOption::ResponseHeadersRead);
	handleProcess(operation, id, callback, udata, streaming);
	create_task(operation, cancellationTokenSource.get_token()).then([=](task<HttpResponseMessage^> response)
	{
		try {
			handleTask(response.get(), id, streaming, callback, udata);
		}
		catch (Exception^ ex)
		{
			handleException(ex, id, callback, udata);
		}
	});
	return id;
}

g_id ghttp_Post(const char* url, const ghttp_Header *header, const void* data, size_t size, int streaming, gevent_Callback callback, void* udata)
{
	std::wstring wstrurl=utf8_ws(url);
	Uri^ uri = ref new Uri(ref new String(wstrurl.c_str()));

	DataWriter ^writer = ref new DataWriter();
	writer->WriteBytes(Platform::ArrayReference<BYTE>((BYTE *)data, (unsigned int)size));
	IBuffer ^postData = writer->DetachBuffer();

	g_id id = g_NextId();
	HttpRequestMessage^ request = ref new HttpRequestMessage(HttpMethod::Post, uri);
	add_headers(header, request);
	HttpBufferContent^ content = ref new HttpBufferContent(postData);
	add_post_headers(header,content);
	request->Content = content;

	IAsyncOperationWithProgress<HttpResponseMessage^, HttpProgress>^ operation = httpClient->SendRequestAsync(request);
	handleProcess(operation, id, callback, udata, streaming);

	create_task(operation, cancellationTokenSource.get_token()).then([=](task<HttpResponseMessage^> response)
	{
		try {
			handleTask(response.get(), id, streaming, callback, udata);
		}
		catch (Exception^ ex)
		{
			handleException(ex, id, callback, udata);
		}
	});
	return id;
}

g_id ghttp_Delete(const char* url, const ghttp_Header *header, int streaming, gevent_Callback callback, void* udata)
{
	std::wstring wstrurl=utf8_ws(url);
	Uri^ uri = ref new Uri(ref new String(wstrurl.c_str()));
	g_id id = g_NextId();

	HttpRequestMessage^ request = ref new HttpRequestMessage(HttpMethod::Delete, uri);
	add_headers(header, request);

	IAsyncOperationWithProgress<HttpResponseMessage^, HttpProgress>^ operation = httpClient->SendRequestAsync(request);
	create_task(operation, cancellationTokenSource.get_token()).then([=](task<HttpResponseMessage^> response)
	{
		try {
			handleTask(response.get(), id, streaming, callback, udata);
		}
		catch (Exception^ ex)
		{
			handleException(ex, id, callback, udata);
		}
	});
	return id;
}

g_id ghttp_Put(const char* url, const ghttp_Header *header, const void* data, size_t size, int streaming, gevent_Callback callback, void* udata)
{
	std::wstring wstrurl=utf8_ws(url);
	Uri^ uri = ref new Uri(ref new String(wstrurl.c_str()));

	DataWriter ^writer = ref new DataWriter();
	writer->WriteBytes(Platform::ArrayReference<BYTE>((BYTE *)data, (unsigned int)size));
	IBuffer ^postData = writer->DetachBuffer();

	g_id id = g_NextId();
	HttpRequestMessage^ request = ref new HttpRequestMessage(HttpMethod::Put, uri);
	add_headers(header, request);
	HttpBufferContent^ content = ref new HttpBufferContent(postData);
	add_post_headers(header, content);
	request->Content = content;

	IAsyncOperationWithProgress<HttpResponseMessage^, HttpProgress>^ operation = httpClient->SendRequestAsync(request);
	create_task(operation, cancellationTokenSource.get_token()).then([=](task<HttpResponseMessage^> response)
	{
		try {
			handleTask(response.get(), id, streaming, callback, udata);
		}
		catch (Exception^ ex)
		{
			handleException(ex, id, callback, udata);
		}
	});
	return id;
}

void ghttp_Close(g_id id)
{

}

void ghttp_CloseAll()
{

}

void ghttp_IgnoreSSLErrors()
{
	if (!sslErrorsIgnore){
		sslErrorsIgnore = true;
		HttpBaseProtocolFilter^ filter = ref new HttpBaseProtocolFilter();
		filter->AllowAutoRedirect = true;
		filter->IgnorableServerCertificateErrors->Append(ChainValidationResult::Expired);
		filter->IgnorableServerCertificateErrors->Append(ChainValidationResult::Untrusted);
		//filter->IgnorableServerCertificateErrors->Append(ChainValidationResult::BasicConstraintsError);
		filter->IgnorableServerCertificateErrors->Append(ChainValidationResult::IncompleteChain);
		//filter->IgnorableServerCertificateErrors->Append(ChainValidationResult::InvalidCertificateAuthorityPolicy);
		filter->IgnorableServerCertificateErrors->Append(ChainValidationResult::InvalidName);
		//filter->IgnorableServerCertificateErrors->Append(ChainValidationResult::InvalidSignature);
		//filter->IgnorableServerCertificateErrors->Append(ChainValidationResult::OtherErrors);
		//filter->IgnorableServerCertificateErrors->Append(ChainValidationResult::RevocationFailure);
		filter->IgnorableServerCertificateErrors->Append(ChainValidationResult::RevocationInformationMissing);
		//filter->IgnorableServerCertificateErrors->Append(ChainValidationResult::Revoked);
		//filter->IgnorableServerCertificateErrors->Append(ChainValidationResult::UnknownCriticalExtension);
		//filter->IgnorableServerCertificateErrors->Append(ChainValidationResult::WrongUsage);
		httpClient = ref new HttpClient(filter);
	}
}

void ghttp_SetProxy(const char *host, int port, const char *user, const char *pass)
{
}

}
