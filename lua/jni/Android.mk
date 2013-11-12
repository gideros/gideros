LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE            := gvfs
LOCAL_SRC_FILES         := ../../libgvfs/libs/$(TARGET_ARCH_ABI)/libgvfs.so

include $(PREBUILT_SHARED_LIBRARY)

###

include $(CLEAR_VARS)

LOCAL_MODULE := lua
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := -O2
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../src $(LOCAL_PATH)/../../libgvfs
LOCAL_SRC_FILES += ../etc/all_lua.c
LOCAL_LDLIBS := -ldl
LOCAL_SHARED_LIBRARIES := gvfs

include $(BUILD_SHARED_LIBRARY)
