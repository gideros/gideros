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

#
# Plugins
#

###

include $(CLEAR_VARS)

LOCAL_MODULE           := controller
LOCAL_ARM_MODE         := arm
LOCAL_CFLAGS           := -O2
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../../Sdk/include $(LOCAL_PATH)/..
LOCAL_SRC_FILES        := controller.cpp controllerbinder.cpp
LOCAL_LDLIBS           := -ldl -llog 
LOCAL_SHARED_LIBRARIES := lua gideros

include $(BUILD_SHARED_LIBRARY)
