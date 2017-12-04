LOCAL_PATH := $(call my-dir)

###

include $(CLEAR_VARS)

LOCAL_MODULE            := lua
LOCAL_SRC_FILES         := ../../../../Sdk/lib/android/$(TARGET_ARCH_ABI)/liblua.so

include $(PREBUILT_SHARED_LIBRARY)

###

include $(CLEAR_VARS)

LOCAL_MODULE            := gideros
LOCAL_SRC_FILES         := ../../../../Sdk/lib/android/$(TARGET_ARCH_ABI)/libgideros.so

include $(PREBUILT_SHARED_LIBRARY)

###

include $(CLEAR_VARS)

INCS:= ../../../Sdk/include
INCS += ../../../Sdk/include/gideros
INCS += ../../../2dsg
INCS += ../../../2dsg/gfxbackends
INCS += ../../../libgideros
INCS += ../../../libgid/include
INCS += ../../../luabinding
INCS += ../../../lua/src

LOCAL_MODULE           := liquidfun
LOCAL_ARM_MODE         := arm
LOCAL_CFLAGS           := -O2
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../Sdk/include $(addprefix $(LOCAL_PATH)/../,$(INCS))
LOCAL_SRC_FILES        := $(addprefix ../common/,lfstatus.cpp liquidfunbinder.cpp lqParticles.cpp lqSprites.cpp lqWorld.cpp)
LOCAL_LDLIBS           := -ldl -llog
LOCAL_STATIC_LIBRARIES := cpufeatures
LOCAL_SHARED_LIBRARIES := lua gideros

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../liquidfun/Box2D
LOCAL_SRC_FILES += \
   	../../../../luabinding/binder.cpp \
   	../../../../2dsg/Matrices.cpp

LOCAL_SRC_FILES += \
	../liquidfun/Box2D/Box2D/Collision/b2BroadPhase.cpp \
	../liquidfun/Box2D/Box2D/Collision/b2CollideCircle.cpp \
	../liquidfun/Box2D/Box2D/Collision/b2CollideEdge.cpp \
	../liquidfun/Box2D/Box2D/Collision/b2CollidePolygon.cpp \
	../liquidfun/Box2D/Box2D/Collision/b2Collision.cpp \
	../liquidfun/Box2D/Box2D/Collision/b2Distance.cpp \
	../liquidfun/Box2D/Box2D/Collision/b2DynamicTree.cpp \
	../liquidfun/Box2D/Box2D/Collision/b2TimeOfImpact.cpp \
	../liquidfun/Box2D/Box2D/Collision/Shapes/b2ChainShape.cpp \
	../liquidfun/Box2D/Box2D/Collision/Shapes/b2CircleShape.cpp \
	../liquidfun/Box2D/Box2D/Collision/Shapes/b2EdgeShape.cpp \
	../liquidfun/Box2D/Box2D/Collision/Shapes/b2PolygonShape.cpp \
	../liquidfun/Box2D/Box2D/Common/b2BlockAllocator.cpp \
	../liquidfun/Box2D/Box2D/Common/b2Draw.cpp \
	../liquidfun/Box2D/Box2D/Common/b2FreeList.cpp \
	../liquidfun/Box2D/Box2D/Common/b2Math.cpp \
	../liquidfun/Box2D/Box2D/Common/b2Settings.cpp \
	../liquidfun/Box2D/Box2D/Common/b2StackAllocator.cpp \
	../liquidfun/Box2D/Box2D/Common/b2Stat.cpp \
	../liquidfun/Box2D/Box2D/Common/b2Timer.cpp \
	../liquidfun/Box2D/Box2D/Common/b2TrackedBlock.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/b2Body.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/b2ContactManager.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/b2Fixture.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/b2Island.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/b2World.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/b2WorldCallbacks.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Contacts/b2CircleContact.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Contacts/b2Contact.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Contacts/b2ContactSolver.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Contacts/b2PolygonContact.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Joints/b2DistanceJoint.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Joints/b2FrictionJoint.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Joints/b2GearJoint.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Joints/b2Joint.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Joints/b2MotorJoint.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Joints/b2MouseJoint.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Joints/b2PrismaticJoint.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Joints/b2PulleyJoint.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Joints/b2RevoluteJoint.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Joints/b2RopeJoint.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Joints/b2WeldJoint.cpp \
	../liquidfun/Box2D/Box2D/Dynamics/Joints/b2WheelJoint.cpp \
	../liquidfun/Box2D/Box2D/Particle/b2Particle.cpp \
	../liquidfun/Box2D/Box2D/Particle/b2ParticleGroup.cpp \
	../liquidfun/Box2D/Box2D/Particle/b2ParticleSystem.cpp \
	../liquidfun/Box2D/Box2D/Particle/b2VoronoiDiagram.cpp \
	../liquidfun/Box2D/Box2D/Rope/b2Rope.cpp

USE_NEON:=
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
USE_NEON:=y
endif
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
USE_NEON:=n
endif
ifeq ($(USE_NEON),y)
LOCAL_SRC_FILES += \
	../liquidfun/Box2D/Box2D/Particle/b2ParticleAssembly.cpp \
	../liquidfun/Box2D/Box2D/Particle/b2ParticleAssembly.neon.s
LOCAL_CFLAGS   += -DLIQUIDFUN_SIMD_NEON -mfpu=neon
endif

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/cpufeatures)
