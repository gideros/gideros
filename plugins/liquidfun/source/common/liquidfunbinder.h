#ifndef BOX2DBINDER2
#define BOX2DBINDER2

#include "binder.h"

#define BIND_LIQUIDFUN 1

class Box2DBinder2
{
public:
	Box2DBinder2(lua_State* L);

private:
	static int loader(lua_State* L);

	static int b2GetScale(lua_State* L);
	static int b2SetScale(lua_State* L);

	static int b2World_create(lua_State* L);
	static int b2World_destruct(lua_State* L);
	static int b2World_CreateBody(lua_State* L);
	static int b2World_Step(lua_State* L);
	static int b2World_DestroyBody(lua_State* L);
	static int b2World_ClearForces(lua_State* L);
	static int b2World_QueryAABB(lua_State* L);
	static int b2World_rayCast(lua_State* L);
	static int b2World_createJoint(lua_State* L);
	static int b2World_destroyJoint(lua_State* L);
	static int b2World_getGravity(lua_State* L);
	static int b2World_setGravity(lua_State* L);
	static int b2World_setDebugDraw(lua_State* L);

	static int b2Body_destruct(lua_State* L);
	//static int b2Body_CreateShape(lua_State* L);
	static int b2Body_CreateFixture(lua_State* L);
	static int b2Body_DestroyFixture(lua_State* L);
	//static int b2Body_SetMassFromShapes(lua_State* L);
	static int b2Body_GetPosition(lua_State* L);
	static int b2Body_SetPosition(lua_State* L);
	static int b2Body_SetAngle(lua_State* L);
	static int b2Body_GetAngle(lua_State* L);
	static int b2Body_GetLinearVelocity(lua_State* L);
	static int b2Body_SetLinearVelocity(lua_State* L);
	//static int b2Body_SetMass(lua_State* L);
	//static int b2Body_SetXForm(lua_State* L);
	//static int b2Body_GetXForm(lua_State* L);
	//static int b2Body_GetWorldCenter(lua_State* L);
	//static int b2Body_GetLocalCenter(lua_State* L);
	static int b2Body_SetAngularVelocity(lua_State* L);
	static int b2Body_GetAngularVelocity(lua_State* L);
	static int b2Body_ApplyForce(lua_State* L);
	static int b2Body_ApplyTorque(lua_State* L);
	static int b2Body_ApplyLinearImpulse(lua_State* L);
	static int b2Body_ApplyAngularImpulse(lua_State* L);
	static int b2Body_isAwake(lua_State* L);
    static int b2Body_setAwake(lua_State* L);
    static int b2Body_setActive(lua_State* L);
	static int b2Body_isActive(lua_State* L);
	static int b2Body_setType(lua_State* L);
	static int b2Body_getType(lua_State* L);
	static int b2Body_getLinearDamping(lua_State* L);
	static int b2Body_setLinearDamping(lua_State* L);
	static int b2Body_getAngularDamping(lua_State* L);
	static int b2Body_setAngularDamping(lua_State* L);
    static int b2Body_getWorldCenter(lua_State* L);
    static int b2Body_getLocalCenter(lua_State* L);
    static int b2Body_getMass(lua_State* L);
    static int b2Body_getInertia(lua_State* L);
    static int b2Body_setGravityScale(lua_State* L);
    static int b2Body_getGravityScale(lua_State* L);
    static int b2Body_getWorldPoint(lua_State* L);
    static int b2Body_getWorldVector(lua_State* L);
    static int b2Body_getLocalPoint(lua_State* L);
    static int b2Body_getLocalVector(lua_State* L);
    static int b2Body_setFixedRotation(lua_State* L);
    static int b2Body_isFixedRotation(lua_State* L);
    static int b2Body_setBullet(lua_State* L);
    static int b2Body_isBullet(lua_State* L);
    static int b2Body_setSleepingAllowed(lua_State* L);
    static int b2Body_isSleepingAllowed(lua_State* L);
    static int b2Body_setTransform(lua_State* L);
    static int b2Body_getTransform(lua_State* L);

	static int b2Fixture_destruct(lua_State* L);
	static int b2Fixture_GetBody(lua_State* L);
	static int b2Fixture_SetSensor(lua_State* L);
	static int b2Fixture_IsSensor(lua_State* L);
	static int b2Fixture_SetFilterData(lua_State* L);
	static int b2Fixture_GetFilterData(lua_State* L);

	static int b2Shape_destruct(lua_State* L);// TODO: buna gerek yok gibi

	static int b2CircleShape_create(lua_State* L);
	static int b2CircleShape_destruct(lua_State* L);
	static int b2CircleShape_set(lua_State* L);

	static int b2PolygonShape_create(lua_State* L);
	static int b2PolygonShape_destruct(lua_State* L);
	static int b2PolygonShape_SetAsBox(lua_State* L);
//	static int b2PolygonShape_SetAsEdge(lua_State* L);
	static int b2PolygonShape_Set(lua_State* L);

	static int b2EdgeShape_create(lua_State* L);
	static int b2EdgeShape_destruct(lua_State* L);
	static int b2EdgeShape_set(lua_State* L);

    static int b2ChainShape_create(lua_State* L);
    static int b2ChainShape_destruct(lua_State* L);
    static int b2ChainShape_createLoop(lua_State* L);
    static int b2ChainShape_createChain(lua_State* L);

/*	static int b2AABB_create(lua_State* L);
	static int b2AABB_destruct(lua_State* L);
	static int b2AABB_GetLowerBound(lua_State* L);
	static int b2AABB_GetUpperBound(lua_State* L);
	static int b2AABB_SetLowerBound(lua_State* L);
	static int b2AABB_SetUpperBound(lua_State* L); */

	static int b2Joint_getType(lua_State* L);
	static int b2Joint_getBodyA(lua_State* L);
	static int b2Joint_getBodyB(lua_State* L);
	static int b2Joint_getAnchorA(lua_State* L);
	static int b2Joint_getAnchorB(lua_State* L);
	static int b2Joint_isActive(lua_State* L);
    static int b2Joint_getReactionForce(lua_State* L);
    static int b2Joint_getReactionTorque(lua_State* L);

	static int b2Joint_destruct(lua_State* L);

	static int getRevoluteJointDef(lua_State* L);
	static int getPrismaticJointDef(lua_State* L);
	static int getDistanceJointDef(lua_State* L);
	static int getPulleyJointDef(lua_State* L);
	static int getMouseJointDef(lua_State* L);
	static int getGearJointDef(lua_State* L);
	static int getWheelJointDef(lua_State* L);
	static int getWeldJointDef(lua_State* L);
	static int getFrictionJointDef(lua_State* L);
    static int getRopeJointDef(lua_State* L);

	static int b2RevoluteJoint_getJointAngle(lua_State* L);
	static int b2RevoluteJoint_getJointSpeed(lua_State* L);
	static int b2RevoluteJoint_isLimitEnabled(lua_State* L);
	static int b2RevoluteJoint_enableLimit(lua_State* L);
	static int b2RevoluteJoint_getLimits(lua_State* L);
	static int b2RevoluteJoint_setLimits(lua_State* L);
	static int b2RevoluteJoint_isMotorEnabled(lua_State* L);
	static int b2RevoluteJoint_enableMotor(lua_State* L);
	static int b2RevoluteJoint_setMotorSpeed(lua_State* L);
	static int b2RevoluteJoint_getMotorSpeed(lua_State* L);
	static int b2RevoluteJoint_setMaxMotorTorque(lua_State* L);
    static int b2RevoluteJoint_getMotorTorque(lua_State* L);

	static int b2PrismaticJoint_getJointTranslation(lua_State* L);
	static int b2PrismaticJoint_getJointSpeed(lua_State* L);
	static int b2PrismaticJoint_isLimitEnabled(lua_State* L);
	static int b2PrismaticJoint_enableLimit(lua_State* L);
	static int b2PrismaticJoint_getLimits(lua_State* L);
	static int b2PrismaticJoint_setLimits(lua_State* L);
	static int b2PrismaticJoint_isMotorEnabled(lua_State* L);
	static int b2PrismaticJoint_enableMotor(lua_State* L);
	static int b2PrismaticJoint_setMotorSpeed(lua_State* L);
	static int b2PrismaticJoint_getMotorSpeed(lua_State* L);
	static int b2PrismaticJoint_setMaxMotorForce(lua_State* L);
    static int b2PrismaticJoint_getMotorForce(lua_State* L);

	static int b2DistanceJoint_setLength(lua_State* L);
	static int b2DistanceJoint_getLength(lua_State* L);
	static int b2DistanceJoint_setFrequency(lua_State* L);
	static int b2DistanceJoint_getFrequency(lua_State* L);
	static int b2DistanceJoint_setDampingRatio(lua_State* L);
	static int b2DistanceJoint_getDampingRatio(lua_State* L);

	static int b2PulleyJoint_getGroundAnchorA(lua_State* L);
	static int b2PulleyJoint_getGroundAnchorB(lua_State* L);
	static int b2PulleyJoint_getLengthA(lua_State* L);
	static int b2PulleyJoint_getLengthB(lua_State* L);
	static int b2PulleyJoint_getRatio(lua_State* L);

	static int b2MouseJoint_setTarget(lua_State* L);
	static int b2MouseJoint_getTarget(lua_State* L);
	static int b2MouseJoint_setMaxForce(lua_State* L);
	static int b2MouseJoint_getMaxForce(lua_State* L);
	static int b2MouseJoint_setFrequency(lua_State* L);
	static int b2MouseJoint_getFrequency(lua_State* L);
	static int b2MouseJoint_setDampingRatio(lua_State* L);
	static int b2MouseJoint_getDampingRatio(lua_State* L);

	static int b2GearJoint_setRatio(lua_State* L);
	static int b2GearJoint_getRatio(lua_State* L);

	static int b2WheelJoint_getJointTranslation(lua_State* L);
	static int b2WheelJoint_getJointSpeed(lua_State* L);
	static int b2WheelJoint_isMotorEnabled(lua_State* L);
	static int b2WheelJoint_enableMotor(lua_State* L);
	static int b2WheelJoint_setMotorSpeed(lua_State* L);
	static int b2WheelJoint_getMotorSpeed(lua_State* L);
	static int b2WheelJoint_setMaxMotorTorque(lua_State* L);
	static int b2WheelJoint_getMaxMotorTorque(lua_State* L);
	static int b2WheelJoint_setSpringFrequencyHz(lua_State* L);
	static int b2WheelJoint_getSpringFrequencyHz(lua_State* L);
	static int b2WheelJoint_setSpringDampingRatio(lua_State* L);
	static int b2WheelJoint_getSpringDampingRatio(lua_State* L);
    static int b2WheelJoint_getMotorTorque(lua_State* L);

    static int b2WeldJoint_setFrequency(lua_State* L);
    static int b2WeldJoint_getFrequency(lua_State* L);
    static int b2WeldJoint_setDampingRatio(lua_State* L);
    static int b2WeldJoint_getDampingRatio(lua_State* L);

	static int b2FrictionJoint_setMaxForce(lua_State* L);
	static int b2FrictionJoint_getMaxForce(lua_State* L);
	static int b2FrictionJoint_setMaxTorque(lua_State* L);
	static int b2FrictionJoint_getMaxTorque(lua_State* L);

    static int b2RopeJoint_setMaxLength(lua_State* L);
    static int b2RopeJoint_getMaxLength(lua_State* L);

	static int b2DebugDraw_create(lua_State* L);
	static int b2DebugDraw_destruct(lua_State* L);
	static int b2DebugDraw_setFlags(lua_State* L);
	static int b2DebugDraw_getFlags(lua_State* L);
	static int b2DebugDraw_appendFlags(lua_State* L);
	static int b2DebugDraw_clearFlags(lua_State* L);

    static int b2Contact_getManifold(lua_State *L);
    static int b2Contact_getWorldManifold(lua_State *L);
    static int b2Contact_isTouching(lua_State *L);
    static int b2Contact_setEnabled(lua_State *L);
    static int b2Contact_isEnabled(lua_State *L);
    static int b2Contact_getFixtureA(lua_State *L);
    static int b2Contact_getChildIndexA(lua_State *L);
    static int b2Contact_getFixtureB(lua_State *L);
    static int b2Contact_getChildIndexB(lua_State *L);
    static int b2Contact_setFriction(lua_State *L);
    static int b2Contact_getFriction(lua_State *L);
    static int b2Contact_resetFriction(lua_State *L);
    static int b2Contact_setRestitution(lua_State *L);
    static int b2Contact_getRestitution(lua_State *L);
    static int b2Contact_resetRestitution(lua_State *L);

    static int testOverlap(lua_State *L);

#if BIND_LIQUIDFUN
    static int b2World_createParticleSystem(lua_State* L);
    static int b2ParticleSystem_createParticle(lua_State* L);
    static int b2ParticleSystem_destroyParticle(lua_State* L);
    static int b2ParticleSystem_createParticleGroup(lua_State* L);
    static int b2ParticleSystem_setTexture(lua_State* L);
    static int b2ParticleSystem_getParticleGroupList(lua_State* L);

    static int b2ParticleGroup_destroyParticles(lua_State* L);
    static int b2ParticleGroup_getParticleCount(lua_State* L);
    static int b2ParticleGroup_containsParticle(lua_State* L);
#endif
};

#endif
