#ifndef LFSTATUS_H
#define LFSTATUS_H

#include <string>

class LFStatus
{
public:
	enum Type
	{
		eArgumentError,
		eRangeError,
		eRuntimeError,
	};

	LFStatus();
	LFStatus(int errorCode, ...);
	LFStatus(int errorCode, const char* arg1);
	LFStatus(int errorCode, const char* arg1, const char* arg2);
	LFStatus(const LFStatus& status);
	~LFStatus();

	LFStatus& operator=(const LFStatus& status);
	
	Type type() const;
	bool error() const;
	const char* errorString() const;
	void clear();

	void swap(LFStatus& other);

private:
	int errorCode_;
	std::string* errorString_;

    void init(int errorCode);
};

#endif
