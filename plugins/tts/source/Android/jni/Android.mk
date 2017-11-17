LOCAL_PATH := $(call my-dir)
GIDEROS_SDK?=../../../../../Sdk
###

include $(CLEAR_VARS)

LOCAL_MODULE            := gvfs
LOCAL_SRC_FILES         := $(GIDEROS_SDK)/lib/android/$(TARGET_ARCH_ABI)/libgvfs.so

include $(PREBUILT_SHARED_LIBRARY)

###

include $(CLEAR_VARS)

LOCAL_MODULE            := lua
LOCAL_SRC_FILES         := $(GIDEROS_SDK)/lib/android/$(TARGET_ARCH_ABI)/liblua.so

include $(PREBUILT_SHARED_LIBRARY)

###

include $(CLEAR_VARS)

LOCAL_MODULE            := gideros
LOCAL_SRC_FILES         := $(GIDEROS_SDK)/lib/android/$(TARGET_ARCH_ABI)/libgideros.so

include $(PREBUILT_SHARED_LIBRARY)

###

include $(CLEAR_VARS)

LOCAL_MODULE           := tts
LOCAL_ARM_MODE         := arm
LOCAL_CFLAGS           := -O2
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/$(GIDEROS_SDK)/include $(LOCAL_PATH)/.. $(LOCAL_PATH)/../../Common
LOCAL_SRC_FILES        := gtts-android.cpp ../../Common/gttsbinder.cpp
LOCAL_LDLIBS           := -ldl -llog -latomic
LOCAL_SHARED_LIBRARIES := gvfs lua gideros

include $(BUILD_SHARED_LIBRARY)
