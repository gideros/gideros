LOCAL_PATH	:= $(call my-dir)/..

include $(CLEAR_VARS)

include $(LOCAL_PATH)/src/Makefile
include $(LOCAL_PATH)/src/loaders/Makefile

SRC_SOURCES	:= $(addprefix src/,$(SRC_OBJS))
LOADERS_SOURCES := $(addprefix src/loaders/,$(LOADERS_OBJS))

LOCAL_MODULE    := xmp
LOCAL_CFLAGS	:= -O3 -DHAVE_MKSTEMP -DHAVE_FNMATCH -DLIBXMP_CORE_PLAYER \
		   -I$(LOCAL_PATH)/include/libxmp-lite -I$(LOCAL_PATH)/src
LOCAL_SRC_FILES := $(SRC_SOURCES:.o=.c.arm) \
		   $(LOADERS_SOURCES:.o=.c)

include $(BUILD_STATIC_LIBRARY)
