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

void GReferenced::setData(void *key, GReferenced *data)
{
    if (data)
        data->ref();

    if (!data_) data_=new std::map<void *, GReferenced*>();

    std::map<void*, GReferenced*>::iterator iter = data_->find(key);
    if (iter != data_->end())
    {
        iter->second->unref();
        data_->erase(iter);
    }

    if (data)
        (*data_)[key] = data;
}

GReferenced* GReferenced::data(void *key) const
{
    if (data_==nullptr) return NULL;
    std::map<void*, GReferenced*>::const_iterator iter = data_->find(key);
    if (iter != data_->end())
        return iter->second;
    return NULL;
}

void GReferenced::setProxy(GReferenced* proxy)
{
	proxy_ = proxy;
}

GReferenced* GReferenced::proxy() const
{
	return proxy_;
}

GReferenced::GReferenced() : refcount_(1), proxy_(NULL)
{
	instanceCount++;
    data_=nullptr;
}

GReferenced::~GReferenced()
{
	assert(refcount_ == 1);

    if (data_!=nullptr) {
        while (!data_->empty())
        {
            void *key = data_->begin()->first;
            setData(key, NULL);
        }
        delete data_;
    }

	instanceCount--;
}
