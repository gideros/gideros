#include "refptr.h"

int GReferenced::instanceCount = 0;

void GReferenced::ref()
{
	++refcount_;
}

void GReferenced::unref()
{
	assert(refcount_ > 0);

	if (--refcount_ == 0)
	{
		refcount_ = 1;
		delete this;
	}
}

int GReferenced::refCount() const
{
	return refcount_;
}

void GReferenced::setData(GReferenced* data)
{
	if (data)
		data->ref();
	if (data_)
		data_->unref();
	data_ = data;
}

GReferenced* GReferenced::data() const
{
	return data_;
}

void GReferenced::setProxy(GReferenced* proxy)
{
	proxy_ = proxy;
}

GReferenced* GReferenced::proxy() const
{
	return proxy_;
}

GReferenced::GReferenced() : refcount_(1), data_(0), proxy_(0)
{
	instanceCount++;
}

GReferenced::~GReferenced()
{
	assert(refcount_ == 1);

	if (data_)
	{
		data_->unref();
		data_ = 0;
	}

	instanceCount--;
}
