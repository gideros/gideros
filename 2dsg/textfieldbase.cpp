#include "textfieldbase.h"
#include "application.h"

bool TextFieldBase::scaleChanged() {
	float scalex = application_->getLogicalScaleX();
	float scaley = application_->getLogicalScaleY();
	FontBase *font=getFont();
	int fontver = (font==NULL)?-1:font->getCacheVersion();
	bool changed=(scalex!=lscalex_)||(scaley!=lscaley_)||(fontver!=lfontCacheVersion_);
	lscalex_=scalex;
	lscaley_=scaley;
	lfontCacheVersion_=fontver;
	return changed;
}

bool TextFieldBase::setDimensions(float w,float h)
{
    bool changed=Sprite::setDimensions(w,h);
    if (changed) {
		layout_.w=w;
		layout_.h=h;
		setLayout(&layout_);
    }
    return changed;
}

void TextFieldBase::getDimensions(float &w,float &h)
{
    w=textwidth_;
    h=textheight_;
}
