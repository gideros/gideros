#include <memory>
#include <concrt.h>
#include <ppltasks.h>

using namespace Windows::System;
using namespace Platform;
using namespace Windows::Storage;


void openUrl(const char* url)
{
	size_t newsize = strlen(url) + 1;
	wchar_t *wcstring = new wchar_t[newsize];

	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wcstring, newsize, url, _TRUNCATE);

	Platform::String ^string = ref new String(wcstring);
	auto uri = ref new Windows::Foundation::Uri(string);
	Launcher::LaunchUriAsync(uri);

	delete[] wcstring;
}

bool canOpenUrl(const char *url)
{
    return true;
}
