#ifndef GID_EVENT_H
#define GID_EVENT_H

#include "stringid.h"
#include "eventtype.h"
#include "gideros_p.h"

class EventDispatcher;
class EventVisitor;

class GIDEROS_API Event
{
public:
	typedef EventType<Event> Type;

	Event(const Type& type) :
		stoppropagation_(false),
		type_(type.type()),
		target_(0)
	{
		uniqueid_ = s_uniqueid_;
		s_uniqueid_++;
	}

	virtual ~Event()
	{

	}

	const char* type() const
	{
		return type_.type();
	}

	int id() const
	{
		return type_.id();
	}

	int uniqueid() const
	{
		return uniqueid_;
	}

	void stopPropagation()
	{
		stoppropagation_ = true;
	}

	bool propagationStopped() const
	{
		return stoppropagation_;
	}

	EventDispatcher* target() const
	{
		return target_;
	}

	static Type ENTER_FRAME;
	static Type EXIT_FRAME;
	static Type SOUND_COMPLETE;
	static Type ADDED_TO_STAGE;
	static Type REMOVED_FROM_STAGE;
//	static Type APPLICATION_DID_FINISH_LAUNCHING;
//	static Type APPLICATION_WILL_TERMINATE;
	static Type MEMORY_WARNING;
	static Type APPLICATION_START;
	static Type APPLICATION_EXIT;
	static Type APPLICATION_SUSPEND;
    static Type APPLICATION_RESUME;
    static Type APPLICATION_BACKGROUND;
    static Type APPLICATION_FOREGROUND;
    static Type APPLICATION_RESIZE;

	virtual void apply(EventVisitor* v);

protected:
	friend class EventDispatcher;

	Event(const char* type) : 
		stoppropagation_(false),
		type_(type),
		target_(0)
	{
		uniqueid_ = s_uniqueid_;
		s_uniqueid_++;
	}

	void setTarget(EventDispatcher* target)
	{
		target_ = target;
	}

private:
	bool stoppropagation_;
	Type type_;
	EventDispatcher* target_;
	static int s_uniqueid_;
	int uniqueid_;
};

class GIDEROS_API OpenUrlEvent : public Event
{
public:
	typedef EventType<OpenUrlEvent> Type;
	OpenUrlEvent(const Type& type,const char *url) : Event(type.type()),	url_(url) {	}
	virtual ~OpenUrlEvent() {}
	const char *url() { return url_.c_str(); }
	virtual void apply(EventVisitor* v);
	static Type OPEN_URL;
private:
	std::string url_;
};

class GIDEROS_API TextInputEvent : public Event
{
public:
	typedef EventType<TextInputEvent> Type;
	TextInputEvent(const Type& type,const char *text,int selStart,int selEnd) : Event(type.type()),	text_(text),selStart_(selStart),selEnd_(selEnd) {	}
	virtual ~TextInputEvent() {}
	const char *text() { return text_.c_str(); }
	int selStart() { return selStart_; }
	int selEnd() { return selEnd_; }
	virtual void apply(EventVisitor* v);
	static Type TEXT_INPUT;
private:
	std::string text_;
	int selStart_;
	int selEnd_;
};


#endif
