#include "gstatus.h"
#include <map>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include "platform.h"

#ifndef _WIN32
#define _snprintf snprintf
#endif


struct GStatusElement
{
    GStatusElement() : errorString(NULL) {}

	GStatusElement(GStatus::Type type, const char* errorString, bool vararg) :
		type(type),
		errorString(errorString),
		vararg(vararg)
	{

	}

	GStatus::Type type;
	const char* errorString;
	bool vararg;
};

static std::map<int, GStatusElement> s_errors;

void GStatus::init(int errorCode)
{
	if (s_errors.empty() == true)
	{
        s_errors[1] = GStatusElement(eRuntimeError, "%s", true);

		s_errors[2150] = GStatusElement(eArgumentError, "An object cannot be added as a child to one of it's children (or children's children, etc.).", false);
		s_errors[2024] = GStatusElement(eArgumentError, "An object cannot be added as a child of itself.", false);
		s_errors[2006] = GStatusElement(eRangeError, "The supplied index is out of bounds.", false);
		s_errors[2008] = GStatusElement(eArgumentError, "Parameter '%s' must be one of the accepted values.", true);
		s_errors[2025] = GStatusElement(eArgumentError, "The supplied Sprite must be a child of the caller.", false);
		s_errors[2009] = GStatusElement(eArgumentError, "Field '%s' must exists.", true);
		s_errors[2010] = GStatusElement(eArgumentError, "Field '%s' must be one of the accepted values.", true);

		s_errors[2100] = GStatusElement(eArgumentError, "Start and end frames must be greater than or equal to 1.", false);
		s_errors[2101] = GStatusElement(eArgumentError, "End frame must be greater than or equal to start frame.", false);
        s_errors[2102] = GStatusElement(eArgumentError, "Timeline array doesn't contain any elements.", false);
        s_errors[2103] = GStatusElement(eArgumentError, "Timeline element is not a table.", false);

		s_errors[5001] = GStatusElement(eRuntimeError, "Body is already destroyed.", false);
		s_errors[5002] = GStatusElement(eRuntimeError, "Fixture is already destroyed.", false);
		s_errors[5003] = GStatusElement(eRuntimeError, "Joint is already destroyed.", false);
		s_errors[5004] = GStatusElement(eRuntimeError, "World is locked.", false);

		s_errors[6000] = GStatusElement(eRuntimeError, "%s: No such file or directory.", true);
        s_errors[6001] = GStatusElement(eRuntimeError, "%s: File is not a PNG file.", true);
        s_errors[6002] = GStatusElement(eRuntimeError, "%s: Error while reading PNG file.", true);
        s_errors[6003] = GStatusElement(eRuntimeError, "%s: PNG color format is not supported. The PNG file should be 24 or 32 bit.", true);
        s_errors[6004] = GStatusElement(eRuntimeError, "%s: Sound format is not recognized.", true);
        s_errors[6005] = GStatusElement(eRuntimeError, "%s: Image format is not supported.", true);
		s_errors[6006] = GStatusElement(eRuntimeError, "%s: File is not a PVR file.", true);
		s_errors[6007] = GStatusElement(eRuntimeError, "%s: PVR pixel type is not supported.", true);
		s_errors[6008] = GStatusElement(eRuntimeError, "%s: File does not contain texture region information.", true);
		s_errors[6009] = GStatusElement(eRuntimeError, "%s: Sound format is not supported.", true);
        s_errors[6010] = GStatusElement(eRuntimeError, "%s: Error while reading JPG file.", true);
        s_errors[6011] = GStatusElement(eRuntimeError, "%s: JPG color format is not supported.", true);
        s_errors[6012] = GStatusElement(eRuntimeError, "%s: Error while reading font file.", true);
        s_errors[6013] = GStatusElement(eRuntimeError, "%s: Error while reading image file.", true);
        s_errors[6014] = GStatusElement(eRuntimeError, "%s: Image color format is not supported.", true);
        s_errors[6015] = GStatusElement(eRuntimeError, "%s: File is not a FNT file.", true);
        s_errors[6016] = GStatusElement(eRuntimeError, "%s: Error while reading FNT file.", true);
        s_errors[6017] = GStatusElement(eArgumentError, "Invalid font size.", false);

		s_errors[7000] = GStatusElement(eRuntimeError, "URL cannot be parsed.", false);
	}
	errorCode_ = errorCode;
    errorString_ = NULL;
}

GStatus::GStatus()
{
    init(0);
}

GStatus::GStatus(int errorCode, ...)
{
	init(errorCode);

	if (s_errors[errorCode].vararg == true)
	{
		va_list args;

		char* buffer = (char*)malloc(1025);
		va_start(args, errorCode);
		vsnprintf(buffer, 1024, s_errors[errorCode].errorString, args);
		va_end(args);
		errorString_ = new std::string(buffer);
		free(buffer);
	}
}

GStatus::GStatus(int errorCode, const char* arg1)
{
	init(errorCode);

	if (s_errors[errorCode].vararg == true)
	{
		char* buffer = (char*)malloc(1025);
		_snprintf(buffer, 1024, s_errors[errorCode].errorString, arg1);
		errorString_ = new std::string(buffer);
		free(buffer);
	}
}

GStatus::GStatus(int errorCode, const char* arg1, const char* arg2)
{
	init(errorCode);

	if (s_errors[errorCode].vararg == true)
	{
		char* buffer = (char*)malloc(1025);
		_snprintf(buffer, 1024, s_errors[errorCode].errorString, arg1, arg2);
		errorString_ = new std::string(buffer);
		free(buffer);
	}
}

GStatus::GStatus(const GStatus& status)
{
	errorCode_ = status.errorCode_;

    if (status.errorString_ == NULL)
        errorString_ = NULL;
	else
		errorString_ = new std::string(*status.errorString_);
}

GStatus::~GStatus()
{
	delete errorString_;
}

GStatus& GStatus::operator=(const GStatus& status)
{
	GStatus(status).swap(*this);
	return *this;
}

GStatus::Type GStatus::type() const
{
	return s_errors[errorCode_].type;
}

bool GStatus::error() const
{
	return errorCode_ != 0;
}

const char* GStatus::errorString() const
{
    if (errorString_ != NULL)
		return errorString_->c_str();

	return s_errors[errorCode_].errorString;
}

void GStatus::clear()
{
	errorCode_ = 0;
	delete errorString_;
    errorString_ = NULL;
}


void GStatus::swap(GStatus& other)
{
	std::swap(errorCode_, other.errorCode_);
	std::swap(errorString_, other.errorString_);
}
