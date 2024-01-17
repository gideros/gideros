//  Created by Nicolas BOUQUET.
//  Copyright � 2017 Gideros Mobile. All rights reserved.
//

#include "pch.h"
#include "sharebinder.h"
#include "gplugin.h"
#include "giderosapi.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::DataTransfer;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Platform;

std::wstring utf8_ws(const char* str);
std::string utf8_us(const wchar_t* str);

static gevent_CallbackList callbackList_;
static void callback_s(int type, void *event, void *udata)
{
  callbackList_.dispatchEvent(type, event);
}

void gshare_Init()
{
}
void gshare_Cleanup()
{
}

bool gshare_Share(std::map<std::string,std::string> values)
{
	auto data_handler = [&](DataTransferManager^, DataRequestedEventArgs^args)
	{
		auto request = args->Request;
		std::map<std::string, std::string>::const_iterator citr = values.begin();
		for (; citr != values.end(); ++citr) {
			const char* mimeType = citr->first.c_str();
			const char* data = citr->second.c_str();
			size_t datasize = citr->second.size();
			if (strstr(mimeType, "image/") == mimeType)
			{
				DataWriter^ writer = ref new DataWriter();
				auto array = Platform::ArrayReference<unsigned char>((unsigned char*)data, (unsigned int)datasize);
				writer->WriteBytes(array);
				IBuffer^ buffer = writer->DetachBuffer();

				InMemoryRandomAccessStream^ randomAccessStream = ref new InMemoryRandomAccessStream();
				randomAccessStream->WriteAsync(buffer);
				randomAccessStream->Seek(0);
				request->Data->SetBitmap(RandomAccessStreamReference::CreateFromStream(randomAccessStream));
			}
			else if (!strcmp(mimeType, "text/html")) {
				request->Data->SetHtmlFormat(ref new String(utf8_ws(data).c_str()));
			}
			else if (!strcmp(mimeType, "text/uri-list")) {
				request->Data->SetWebLink(ref new Uri(ref new String(utf8_ws(data).c_str())));
			}
			else if (strstr(mimeType, "text/") == mimeType)
			{
				request->Data->SetText(ref new String(utf8_ws(data).c_str()));
			}
		}
	};

	gdr_dispatchUi([&] {
		DataTransferManager^ dataTransferManager = DataTransferManager::GetForCurrentView();
		auto eventHandler=ref new TypedEventHandler<DataTransferManager^, DataRequestedEventArgs^>(data_handler);
		auto token = dataTransferManager->DataRequested += eventHandler;
		DataTransferManager::ShowShareUI();
		dataTransferManager->DataRequested -= token;
	}, true);
	return true;

}


bool gshare_Import(const char *mime, const char *extension)
{
	return false;
}

bool gshare_Export(const char *data,size_t dataSize,const char *mime, const char *filename)
{
	return false;
}


g_id gshare_AddCallback(gevent_Callback callback, void *udata)
{
    return callbackList_.addCallback(callback, udata);
}

void gshare_RemoveCallback(gevent_Callback callback, void *udata)
{
    callbackList_.removeCallback(callback, udata);
}


int gshare_Capabilities()
{
	return CAP_SHARE;
}

