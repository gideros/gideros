#ifndef GREFERENCED_H
#define GREFERENCED_H

#include <vector>
#include <set>
#include <map>
#include <cassert>
#include <string>
#include "gexport.h"

class GIDEROS_API GReferenced
{
public:
	void ref();
	void unref();
	int refCount() const;

    void setData(void *key, GReferenced* data);
    GReferenced* data(void *key) const;

	void setProxy(GReferenced* proxy);
	GReferenced* proxy() const;

	static int instanceCount;

protected:
	GReferenced();
	virtual ~GReferenced();

private:
	int refcount_;
    std::map<void *, GReferenced*> data_;
	GReferenced* proxy_;

private:
	GReferenced(const GReferenced&);
	GReferenced& operator=(const GReferenced&);
};

#endif
