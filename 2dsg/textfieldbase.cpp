#include "textfieldbase.h"
#include "application.h"

bool TextFieldBase::scaleChanged() {
	float scalex = application_->getLogicalScaleX();
	float scaley = application_->getLogicalScaleY();
	bool changed=(scalex!=lscalex_)||(scaley!=lscaley_);
	lscalex_=scalex;
	lscaley_=scaley;
	return changed;
}
