#ifndef GREFERENCED_H
#define GREFERENCED_H

#include <vector>
#include <set>
#include <cassert>
#include <string>
#include "gexport.h"

class GIDEROS_API GReferenced
{
public:
	void ref();
	void unref();
	int refCount() const;

	void setData(GReferenced* data);
	GReferenced* data() const;

	void setProxy(GReferenced* proxy);
	GReferenced* proxy() const;

	static int instanceCount;

protected:
	GReferenced();
	virtual ~GReferenced();

private:
	int refcount_;
	GReferenced* data_;
	GReferenced* proxy_;

private:
	GReferenced(const GReferenced&);
	GReferenced& operator=(const GReferenced&);
};

#endif
