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

LOCAL_MODULE           := lualanes
LOCAL_ARM_MODE         := arm
LOCAL_CFLAGS           := -O2
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../Sdk/include
LOCAL_SRC_FILES        := ../custom_alloc.c \
    					  ../lanes.c \
    					  ../compat.c \
    					  ../threading.c \
    					  ../tools.c \
    					  ../deep.c \
    					  ../keeper.c \
    					  ../universe.c
						  ../lualanes_stub.cpp
LOCAL_LDLIBS           := -ldl -llog
LOCAL_SHARED_LIBRARIES := lua gideros

include $(BUILD_SHARED_LIBRARY)
