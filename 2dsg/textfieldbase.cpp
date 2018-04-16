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

void TextFieldBase::setDimensions(float w,float h)
{
    Sprite::setDimensions(w,h);
    layout_.w=w;
    layout_.h=h;
    setLayout(&layout_);
}

void TextFieldBase::getDimensions(float &w,float &h)
{
    w=textwidth_;
    h=textheight_;
}
