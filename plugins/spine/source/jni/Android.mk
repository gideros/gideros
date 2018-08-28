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

LOCAL_MODULE            := gvfs
LOCAL_SRC_FILES         := ../../../../Sdk/lib/android/$(TARGET_ARCH_ABI)/libgvfs.so

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

LOCAL_MODULE           := spine
LOCAL_ARM_MODE         := arm
LOCAL_CFLAGS           := -O2
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../Sdk/include $(addprefix $(LOCAL_PATH)/../,$(INCS))
LOCAL_SRC_FILES        := $(addprefix ../Common/,spinebinder.cpp)
LOCAL_LDLIBS           := -ldl -llog 
LOCAL_SHARED_LIBRARIES := lua gideros gvfs

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../spine-runtimes/spine-c/spine-c/include
LOCAL_SRC_FILES += \
   	../../../../luabinding/binder.cpp \
  	$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/../../spine-runtimes/spine-c/spine-c/src/spine/*.c))

include $(BUILD_SHARED_LIBRARY)
