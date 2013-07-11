#ifndef GIDEROSEXCEPTION_H
#define GIDEROSEXCEPTION_H

#include "gstatus.h"

class GiderosException : public std::exception
{
public:
    GiderosException(const GStatus &status) throw() : status_(status)
	{
	}

	virtual ~GiderosException() throw()
	{

	}

    virtual const char *what() const throw()
	{
		return status_.errorString();
	}

    const GStatus &status() const
    {
        return status_;
    }

private:
	GStatus status_;
};


#endif
