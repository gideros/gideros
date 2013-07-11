#ifndef GIDEROS_P_H
#define GIDEROS_P_H

#include "gexport.h"


class LuaApplicationBase
{
public:
	virtual ~LuaApplicationBase() {}

	virtual void initialize() = 0;
	virtual void deinitialize() = 0;

    virtual void setError(const char* error) = 0;
    virtual bool isErrorSet() const = 0;
    virtual const char* getError() const = 0;
    virtual void clearError() = 0;
};


#endif
