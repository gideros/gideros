#include "textfieldbase.h"
#include <utf8.h>

void TextFieldBase::updateWide()
{
	size_t wsize = utf8_to_wchar(text_.c_str(), text_.size(), NULL, 0, 0);

	if (wsize == 0)
	{
		wtext_.clear();
		return;
	}

	wtext_.resize(wsize);
	utf8_to_wchar(text_.c_str(), text_.size(), &wtext_[0], wsize, 0);
}
