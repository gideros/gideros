#ifndef GSTATUS_H
#define GSTATUS_H

#include <string>

class GStatus
{
public:
	enum Type
	{
		eArgumentError,
		eRangeError,
		eRuntimeError,
	};

	GStatus();
	GStatus(int errorCode, ...);
	GStatus(int errorCode, const char* arg1);
	GStatus(int errorCode, const char* arg1, const char* arg2);
	GStatus(const GStatus& status);
	~GStatus();

	GStatus& operator=(const GStatus& status);
	
	Type type() const;
	bool error() const;
	const char* errorString() const;
	void clear();

	void swap(GStatus& other);

private:
	int errorCode_;
	std::string* errorString_;

    void init(int errorCode);
};

#endif
