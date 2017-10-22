/*
 * lqWorld.h
 *
 *  Created on: 22 oct. 2017
 *      Author: Nicolas
 */

#ifndef LQWORLD_H_
#define LQWORLD_H_
#include "liquidfunbinder.h"
#include <eventdispatcher.h>
#include <event.h>

#include <string>
class b2DebugDraw;
class b2WorldED : public b2World, public EventDispatcher
{
public:
	b2WorldED(const b2Vec2& gravity, bool doSleep);
	virtual ~b2WorldED();

	void SetDestructionListener(b2DestructionListener* listener)
	{
		m_destructionListener = listener;
		b2World::SetDestructionListener(listener);
	}

	b2DestructionListener* GetDestructionListener() const
	{
		return m_destructionListener;
	}

	void SetContactListener(b2ContactListener* listener)
	{
		m_contactListener = listener;
		b2World::SetContactListener(listener);
	}

	b2ContactListener* GetContactListener() const
	{
		return m_contactListener;
	}

	b2DebugDraw* GetDebugDraw()
	{
		return m_debugDraw;
	}

	void SetDebugDraw(b2DebugDraw* draw);

	static Event::Type BEGIN_CONTACT;
	static Event::Type END_CONTACT;
    static Event::Type BEGIN_CONTACT_PARTICLE;
    static Event::Type END_CONTACT_PARTICLE;
    static Event::Type BEGIN_CONTACT_PARTICLE2;
    static Event::Type END_CONTACT_PARTICLE2;
	static Event::Type PRE_SOLVE;
	static Event::Type POST_SOLVE;

	std::string error;

private:
	b2DestructionListener* m_destructionListener;
	b2ContactListener* m_contactListener;
	b2DebugDraw* m_debugDraw;
};



#endif /* LQWORLD_H_ */
