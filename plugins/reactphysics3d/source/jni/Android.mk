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

####

include $(CLEAR_VARS)

INCS:= ../../../Sdk/include
INCS += ../../../Sdk/include/gideros
INCS += ../../../2dsg
INCS += ../../../2dsg/gfxbackends
INCS += ../../../libgideros
INCS += ../../../libgid/include
INCS += ../../../luabinding
INCS += ../../../lua/src
INCS += ../reactphysics3d/src

LOCAL_MODULE           := reactphysics3d
LOCAL_ARM_MODE         := arm
LOCAL_CFLAGS           := -O2
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../Sdk/include $(addprefix $(LOCAL_PATH)/../,$(INCS))
LOCAL_LDLIBS           := -ldl -llog
LOCAL_STATIC_LIBRARIES := 
LOCAL_SHARED_LIBRARIES := lua gideros


FSRCS := Common/reactbinder.cpp \
 	 	../../../luabinding/binder.cpp \
  		../reactphysics3d/src/body/Body.cpp \
  		../reactphysics3d/src/body/CollisionBody.cpp \
  		../reactphysics3d/src/body/RigidBody.cpp \
  		../reactphysics3d/src/collision/CollisionCallback.cpp \
  		../reactphysics3d/src/collision/CollisionDetection.cpp \
  		../reactphysics3d/src/collision/ContactManifold.cpp \
  		../reactphysics3d/src/collision/ContactManifoldInfo.cpp \
  		../reactphysics3d/src/collision/ContactManifoldSet.cpp \
  		../reactphysics3d/src/collision/HalfEdgeStructure.cpp \
  		../reactphysics3d/src/collision/MiddlePhaseTriangleCallback.cpp \
  		../reactphysics3d/src/collision/NarrowPhaseInfo.cpp \
  		../reactphysics3d/src/collision/PolygonVertexArray.cpp \
  		../reactphysics3d/src/collision/PolyhedronMesh.cpp \
  		../reactphysics3d/src/collision/ProxyShape.cpp \
  		../reactphysics3d/src/collision/RaycastInfo.cpp \
  		../reactphysics3d/src/collision/TriangleMesh.cpp \
  		../reactphysics3d/src/collision/TriangleVertexArray.cpp \
  		../reactphysics3d/src/collision/broadphase/BroadPhaseAlgorithm.cpp \
  		../reactphysics3d/src/collision/broadphase/DynamicAABBTree.cpp \
  		../reactphysics3d/src/collision/narrowphase/CapsuleVsCapsuleAlgorithm.cpp \
  		../reactphysics3d/src/collision/narrowphase/CapsuleVsConvexPolyhedronAlgorithm.cpp \
  		../reactphysics3d/src/collision/narrowphase/ConvexPolyhedronVsConvexPolyhedronAlgorithm.cpp \
  		../reactphysics3d/src/collision/narrowphase/DefaultCollisionDispatch.cpp \
  		../reactphysics3d/src/collision/narrowphase/SphereVsCapsuleAlgorithm.cpp \
  		../reactphysics3d/src/collision/narrowphase/SphereVsConvexPolyhedronAlgorithm.cpp \
  		../reactphysics3d/src/collision/narrowphase/SphereVsSphereAlgorithm.cpp \
  		../reactphysics3d/src/collision/narrowphase/GJK/GJKAlgorithm.cpp \
  		../reactphysics3d/src/collision/narrowphase/GJK/VoronoiSimplex.cpp \
  		../reactphysics3d/src/collision/narrowphase/SAT/SATAlgorithm.cpp \
  		../reactphysics3d/src/collision/shapes/AABB.cpp \
  		../reactphysics3d/src/collision/shapes/BoxShape.cpp \
  		../reactphysics3d/src/collision/shapes/CapsuleShape.cpp \
  		../reactphysics3d/src/collision/shapes/CollisionShape.cpp \
  		../reactphysics3d/src/collision/shapes/ConcaveMeshShape.cpp \
  		../reactphysics3d/src/collision/shapes/ConcaveShape.cpp \
  		../reactphysics3d/src/collision/shapes/ConvexMeshShape.cpp \
  		../reactphysics3d/src/collision/shapes/ConvexPolyhedronShape.cpp \
  		../reactphysics3d/src/collision/shapes/ConvexShape.cpp \
  		../reactphysics3d/src/collision/shapes/HeightFieldShape.cpp \
  		../reactphysics3d/src/collision/shapes/SphereShape.cpp \
  		../reactphysics3d/src/collision/shapes/TriangleShape.cpp \
  		../reactphysics3d/src/constraint/BallAndSocketJoint.cpp \
  		../reactphysics3d/src/constraint/ContactPoint.cpp \
  		../reactphysics3d/src/constraint/FixedJoint.cpp \
  		../reactphysics3d/src/constraint/HingeJoint.cpp \
  		../reactphysics3d/src/constraint/Joint.cpp \
  		../reactphysics3d/src/constraint/SliderJoint.cpp \
  		../reactphysics3d/src/engine/CollisionWorld.cpp \
  		../reactphysics3d/src/engine/ConstraintSolver.cpp \
  		../reactphysics3d/src/engine/ContactSolver.cpp \
  		../reactphysics3d/src/engine/DynamicsWorld.cpp \
  		../reactphysics3d/src/engine/Island.cpp \
  		../reactphysics3d/src/engine/Material.cpp \
  		../reactphysics3d/src/engine/OverlappingPair.cpp \
  		../reactphysics3d/src/engine/Timer.cpp \
  		../reactphysics3d/src/mathematics/mathematics_functions.cpp \
  		../reactphysics3d/src/mathematics/Matrix2x2.cpp \
  		../reactphysics3d/src/mathematics/Matrix3x3.cpp \
  		../reactphysics3d/src/mathematics/Quaternion.cpp \
  		../reactphysics3d/src/mathematics/Transform.cpp \
  		../reactphysics3d/src/mathematics/Vector2.cpp \
  		../reactphysics3d/src/mathematics/Vector3.cpp \
  		../reactphysics3d/src/memory/DefaultPoolAllocator.cpp \
  		../reactphysics3d/src/memory/DefaultSingleFrameAllocator.cpp \
  		../reactphysics3d/src/memory/MemoryManager.cpp \
  		../reactphysics3d/src/utils/Logger.cpp \
  		../reactphysics3d/src/utils/Profiler.cpp 
LOCAL_SRC_FILES :=$(addprefix ../,$(FSRCS)) 

include $(BUILD_SHARED_LIBRARY)
