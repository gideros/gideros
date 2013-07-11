#ifndef EVENTTYPE_H
#define EVENTTYPE_H

template <class T>
class EventType
{
public:
	EventType(const char* type) : type_(type), id_(-1)
	{

	}

	const char* type() const
	{
		return type_;
	}

	int id() const
	{
		if (id_ == -1)
			id_ = StringId::instance().id(type_);

		return id_;
	}

private:
	const char* type_;
	mutable int id_;
};

#endif
