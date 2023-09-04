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
	bool isOfType(int type) const { return (typeSig_==0xF00DCAFE)&&((typeMap_&type)==type); };
	void setTypeMap(int typemap) { typeMap_=typemap; typeSig_=0xF00DCAFE; };

	static int instanceCount;

protected:
	GReferenced();
	virtual ~GReferenced();

private:
	int refcount_;
	int typeMap_;
    std::map<void *, GReferenced*> *data_;
    unsigned int typeSig_;
    GReferenced* proxy_;

private:
	GReferenced(const GReferenced&);
	GReferenced& operator=(const GReferenced&);
};

#define GREFERENCED_TYPEMAP_EVENTDISPATCHER	0b0000000000000001
#define GREFERENCED_TYPEMAP_SPRITE			0b0000000000000011
#define GREFERENCED_TYPEMAP_PIXEL			0b0000000000000111

#endif
