LOCAL_PATH := $(call my-dir)

#
# Gideros Shared Library
# 

###

include $(CLEAR_VARS)

LOCAL_MODULE            := lua
LOCAL_SRC_FILES         := ../../../../../Sdk/lib/android/$(TARGET_ARCH_ABI)/liblua.so

include $(PREBUILT_SHARED_LIBRARY)

###

include $(CLEAR_VARS)

LOCAL_MODULE            := gideros
LOCAL_SRC_FILES         := ../../../../../Sdk/lib/android/$(TARGET_ARCH_ABI)/libgideros.so

include $(PREBUILT_SHARED_LIBRARY)

###

include $(CLEAR_VARS)

LOCAL_MODULE            := gvfs
LOCAL_SRC_FILES         := ../../../../../Sdk/lib/android/$(TARGET_ARCH_ABI)/libgvfs.so

include $(PREBUILT_SHARED_LIBRARY)

###

#
# Plugins
#

###

include $(CLEAR_VARS)

LOCAL_MODULE           := camera
LOCAL_ARM_MODE         := arm
LOCAL_CFLAGS           := -O2
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../../Sdk/include $(LOCAL_PATH)/../../common
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../../libgid/include
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../../libgideros
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../../luabinding
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../../lua/src
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../../2dsg
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../../2dsg/gfxbackends
LOCAL_SRC_FILES        := ../../common/camerabinder.cpp gcameraplugin.cpp
LOCAL_LDLIBS           := -ldl -llog -lGLESv2 
LOCAL_SHARED_LIBRARIES := lua gideros gvfs

include $(BUILD_SHARED_LIBRARY)