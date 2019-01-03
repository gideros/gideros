#ifndef GPROXY_H
#define GPROXY_H

#include "gexport.h"
#include "greferenced.h"

class GIDEROS_API GProxy : public GReferenced
{
protected:
	enum GType
	{
		eBase,
		eEventDispatcher,
	};

public:
	GProxy(GType type = eBase);
	virtual ~GProxy();

	GReferenced* object() const
	{
		return object_;
	}

protected:
	GReferenced* object_;
};

class GIDEROS_API GEventDispatcherProxy : public GProxy
{
public:
	G_KEEP GEventDispatcherProxy(GType type = eEventDispatcher);
	G_KEEP virtual ~GEventDispatcherProxy();
};


#endif
