#include <map>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>

#include "lfstatus.h"
#include "platform.h"

#ifndef _WIN32
#define _snprintf snprintf
#endif


struct LFStatusElement
{
    LFStatusElement() : errorString(NULL) {}

	LFStatusElement(LFStatus::Type type, const char* errorString, bool vararg) :
		type(type),
		errorString(errorString),
		vararg(vararg)
	{

	}

	LFStatus::Type type;
	const char* errorString;
	bool vararg;
};

static std::map<int, LFStatusElement> s_errors;

void LFStatus::init(int errorCode)
{
	if (s_errors.empty() == true)
	{
        s_errors[1] = LFStatusElement(eRuntimeError, "%s", true);

		s_errors[2150] = LFStatusElement(eArgumentError, "An object cannot be added as a child to one of it's children (or children's children, etc.).", false);
		s_errors[2024] = LFStatusElement(eArgumentError, "An object cannot be added as a child of itself.", false);
		s_errors[2006] = LFStatusElement(eRangeError, "The supplied index is out of bounds.", false);
		s_errors[2008] = LFStatusElement(eArgumentError, "Parameter '%s' must be one of the accepted values.", true);
		s_errors[2025] = LFStatusElement(eArgumentError, "The supplied Sprite must be a child of the caller.", false);
		s_errors[2009] = LFStatusElement(eArgumentError, "Field '%s' must exists.", true);
		s_errors[2010] = LFStatusElement(eArgumentError, "Field '%s' must be one of the accepted values.", true);

		s_errors[5001] = LFStatusElement(eRuntimeError, "Body is already destroyed.", false);
		s_errors[5002] = LFStatusElement(eRuntimeError, "Fixture is already destroyed.", false);
		s_errors[5003] = LFStatusElement(eRuntimeError, "Joint is already destroyed.", false);
		s_errors[5004] = LFStatusElement(eRuntimeError, "World is locked.", false);
		s_errors[5005] = LFStatusElement(eRuntimeError, "Group is already destroyed.", false);
	}
	errorCode_ = errorCode;
    errorString_ = NULL;
}

LFStatus::LFStatus()
{
    init(0);
}

LFStatus::LFStatus(int errorCode, ...)
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

LFStatus::LFStatus(int errorCode, const char* arg1)
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

LFStatus::LFStatus(int errorCode, const char* arg1, const char* arg2)
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

LFStatus::LFStatus(const LFStatus& status)
{
	errorCode_ = status.errorCode_;

    if (status.errorString_ == NULL)
        errorString_ = NULL;
	else
		errorString_ = new std::string(*status.errorString_);
}

LFStatus::~LFStatus()
{
	delete errorString_;
}

LFStatus& LFStatus::operator=(const LFStatus& status)
{
	LFStatus(status).swap(*this);
	return *this;
}

LFStatus::Type LFStatus::type() const
{
	return s_errors[errorCode_].type;
}

bool LFStatus::error() const
{
	return errorCode_ != 0;
}

const char* LFStatus::errorString() const
{
    if (errorString_ != NULL)
		return errorString_->c_str();

	return s_errors[errorCode_].errorString;
}

void LFStatus::clear()
{
	errorCode_ = 0;
	delete errorString_;
    errorString_ = NULL;
}


void LFStatus::swap(LFStatus& other)
{
	std::swap(errorCode_, other.errorCode_);
	std::swap(errorString_, other.errorString_);
}
