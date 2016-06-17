#include <ghttp.h>
#include "collection.h"
#include <ppltasks.h>
#include <map>
using namespace concurrency;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Headers;
using namespace Windows::Web::Http::Filters;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Streams;
using namespace Windows::Security::Cryptography::Certificates;
using namespace Platform;

static HttpClient^ httpClient;
static cancellation_token_source cancellationTokenSource;
static bool sslErrorsIgnore = false;
static std::map<g_id, std::vector<unsigned char> > map;
//static std::vector<unsigned char> dataRead;
task<IBuffer^> readData(IInputStream^ stream, g_id id)
{
	// Do an asynchronous read. We need to use use_current() with the continuations since the tasks are completed on
	// background threads and we need to run on the UI thread to update the UI.
	Buffer^ readBuffer = ref new Buffer(1000);
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
		map[id].insert(map[id].end(), first, end);

		// Continue reading until the response is complete.  When done, return previousTask that is complete.
		bool more = buffer->Length;
		delete buffer;
		return more ? readData(stream, id) : readTask;
	});
}


void handleException(Exception^ ex, g_id id, gevent_Callback callback, void* udata)
{
	ghttp_ErrorEvent *event = (ghttp_ErrorEvent*)malloc(sizeof(ghttp_ErrorEvent));
	//std::wstring werror(ex->Message->Begin());
	//std::string strerror(werror.begin(), werror.end());
	//strerror.c_str();
	gevent_EnqueueEvent(id, callback, GHTTP_ERROR_EVENT, event, 1, udata);
	map.erase(id);
}

void handleProcess(IAsyncOperationWithProgress<HttpResponseMessage^, HttpProgress>^ operation, g_id id, gevent_Callback callback, void* udata){
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

				gevent_EnqueueEvent(id, callback, GHTTP_PROGRESS_EVENT, event, 1, udata);
			}
		}
		else
		{
			return;
		}
	});
}

void handleTask(HttpResponseMessage^ response, g_id id, gevent_Callback callback, void* udata){
	map[id] = std::vector<unsigned char>();
	create_task(response->Content->ReadAsInputStreamAsync(), cancellationTokenSource.get_token()).then([=](task<IInputStream^> previousTask)
	{
		IInputStream^ stream = previousTask.get();
		return readData(stream, id);
	}).then([=](task<IBuffer^> previousTask)
	{
		try
		{
			// Check if any previous task threw an exception.
			previousTask.get();


			//progress finished
			ghttp_ProgressEvent* eventp = (ghttp_ProgressEvent*)malloc(sizeof(ghttp_ProgressEvent));
			eventp->bytesLoaded = map[id].size();
			eventp->bytesTotal = map[id].size();

			gevent_EnqueueEvent(id, callback, GHTTP_PROGRESS_EVENT, eventp, 1, udata);
			
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
			ghttp_ResponseEvent *event = (ghttp_ResponseEvent*)malloc(sizeof(ghttp_ResponseEvent) + sizeof(ghttp_Header)*hdrCount + map[id].size() + hdrSize);

			event->data = (char*)event + sizeof(ghttp_ResponseEvent) + sizeof(ghttp_Header)*hdrCount;
			memcpy(event->data, map[id].data(), map[id].size());
			event->size = map[id].size();

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
				std::string strkey(wkey.begin(), wkey.end());
				memcpy(hdrData, strkey.c_str(), ds);
				event->headers[hdrn].name = hdrData;
				hdrData += ds;
				*(hdrData++) = 0;
				ds = pair->Value->Length();
				std::wstring wval(pair->Value->Begin());
				std::string strval(wval.begin(), wval.end());
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
				std::string strkey(wkey.begin(), wkey.end());
				memcpy(hdrData, strkey.c_str(), ds);
				event->headers[hdrn].name = hdrData;
				hdrData += ds;
				*(hdrData++) = 0;
				ds = pair->Value->Length();
				std::wstring wval(pair->Value->Begin());
				std::string strval(wval.begin(), wval.end());
				memcpy(hdrData, strval.c_str(), ds);
				event->headers[hdrn].value = hdrData;
				hdrData += ds;
				*(hdrData++) = 0;
				hdrn++;
			}
			event->headers[hdrn].name = NULL;
			event->headers[hdrn].value = NULL;

			gevent_EnqueueEvent(id, callback, GHTTP_RESPONSE_EVENT, event, 1, udata);
			map.erase(id);

		}
		catch (const task_canceled&)
		{
			ghttp_ErrorEvent *event = (ghttp_ErrorEvent*)malloc(sizeof(ghttp_ErrorEvent));
			gevent_EnqueueEvent(id, callback, GHTTP_ERROR_EVENT, event, 1, udata);
			map.erase(id);
		}
		catch (Exception^ ex)
		{
			handleException(ex, id, callback, udata);
		}
	});
}

void add_headers(const ghttp_Header *header){
	HttpRequestHeaderCollection ^headers = httpClient->DefaultRequestHeaders;
	headers->UserAgent->ParseAdd("Gideros");
	if (header){
		for (; header->name; ++header){
			std::string strkey(header->name);
			std::wstring wstrkey(strkey.begin(), strkey.end());
			std::string strval(header->value);
			std::wstring wstrval(strval.begin(), strval.end());
			if (_strnicmp(header->name, "content-", 8))
				headers->Insert(ref new String(wstrkey.c_str()), ref new String(wstrval.c_str()));
		}
	}
}

void add_post_headers(const ghttp_Header *header, HttpStringContent^content){
	if (header){
		for (; header->name; ++header){
			std::string strkey(header->name);
			std::wstring wstrkey(strkey.begin(), strkey.end());
			std::string strval(header->value);
			std::wstring wstrval(strval.begin(), strval.end());
			if (!_stricmp(header->name, "content-type"))
				content->Headers->ContentType=ref new HttpMediaTypeHeaderValue(ref new String(wstrval.c_str()));
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

g_id ghttp_Get(const char* url, const ghttp_Header *header, gevent_Callback callback, void* udata)
{
	std::string strurl(url);
	std::wstring wstrurl(strurl.begin(), strurl.end());
	Uri^ uri = ref new Uri(ref new String(wstrurl.c_str()));
	g_id id = g_NextId();
	add_headers(header);

	IAsyncOperationWithProgress<HttpResponseMessage^, HttpProgress>^ operation = httpClient->GetAsync(uri, HttpCompletionOption::ResponseHeadersRead);
	handleProcess(operation, id, callback, udata);
	create_task(operation, cancellationTokenSource.get_token()).then([=](task<HttpResponseMessage^> response)
	{
		try {
			handleTask(response.get(), id, callback, udata);
		}
		catch (Exception^ ex)
		{
			handleException(ex, id, callback, udata);
		}
	});
	return id;
}

g_id ghttp_Post(const char* url, const ghttp_Header *header, const void* data, size_t size, gevent_Callback callback, void* udata)
{
	std::string strurl(url);
	std::wstring wstrurl(strurl.begin(), strurl.end());
	Uri^ uri = ref new Uri(ref new String(wstrurl.c_str()));

	String^ postData = "";
	if (size > 0){
		std::string strdata((const char*)data);
		std::wstring wdata(strdata.begin(), strdata.end());
		postData = ref new String(wdata.c_str());
	}
	g_id id = g_NextId();


	HttpStringContent^ content = ref new HttpStringContent(postData);
	add_headers(header);
	add_post_headers(header,content);

	IAsyncOperationWithProgress<HttpResponseMessage^, HttpProgress>^ operation = httpClient->PostAsync(uri, content);
	handleProcess(operation, id, callback, udata);

	create_task(operation, cancellationTokenSource.get_token()).then([=](task<HttpResponseMessage^> response)
	{
		try {
			handleTask(response.get(), id, callback, udata);
		}
		catch (Exception^ ex)
		{
			handleException(ex, id, callback, udata);
		}
	});
	return id;
}

g_id ghttp_Delete(const char* url, const ghttp_Header *header, gevent_Callback callback, void* udata)
{
	std::string strurl(url);
	std::wstring wstrurl(strurl.begin(), strurl.end());
	Uri^ uri = ref new Uri(ref new String(wstrurl.c_str()));
	g_id id = g_NextId();

	add_headers(header);

	create_task(httpClient->DeleteAsync(uri), cancellationTokenSource.get_token()).then([=](task<HttpResponseMessage^> response)
	{
		try {
			handleTask(response.get(), id, callback, udata);
		}
		catch (Exception^ ex)
		{
			handleException(ex, id, callback, udata);
		}
	});
	return id;
}

g_id ghttp_Put(const char* url, const ghttp_Header *header, const void* data, size_t size, gevent_Callback callback, void* udata)
{
	std::string strurl(url);
	std::wstring wstrurl(strurl.begin(), strurl.end());
	Uri^ uri = ref new Uri(ref new String(wstrurl.c_str()));
	String^ postData = "";
	if (size > 0){
		std::string strdata((const char*)data);
		std::wstring wdata(strdata.begin(), strdata.end());
		postData = ref new String(wdata.c_str());
	}
	g_id id = g_NextId();

	HttpStringContent^ content = ref new HttpStringContent(postData);
	add_headers(header);
	add_post_headers(header, content);

	create_task(httpClient->PutAsync(uri, content), cancellationTokenSource.get_token()).then([=](task<HttpResponseMessage^> response)
	{
		try {
			handleTask(response.get(), id, callback, udata);
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
