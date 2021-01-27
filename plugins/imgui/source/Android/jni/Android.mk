LOCAL_PATH := $(call my-dir)

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

include $(CLEAR_VARS)

LOCAL_MODULE            := gvfs
LOCAL_SRC_FILES         := ../../../../../Sdk/lib/android/$(TARGET_ARCH_ABI)/libgvfs.so

include $(PREBUILT_SHARED_LIBRARY)

###

include $(CLEAR_VARS)


INCS := ../../../../Sdk/include
INCS += ../../../../Sdk/include/gideros
INCS += ../../../../2dsg
INCS += ../../../../2dsg/gfxbackends
INCS += ../../../../libgideros
INCS += ../../../../libgid/include
INCS += ../../../../luabinding
INCS += ../../../../lua/src
INCS += ../Common/imgui_src
INCS += ../Common

LSOURCES := \
        ../Common/imgui_src/imgui.cpp \
        ../Common/imgui_src/imgui_demo.cpp \
        ../Common/imgui_src/imgui_draw.cpp \
        ../Common/imgui_src/imgui_widgets.cpp \
        ../Common/imgui_src/imgui_tables.cpp \
        ../Common/imgui_user.cpp \
	   	../../../../2dsg/Matrices.cpp \
	   	../../../../2dsg/mouseevent.cpp \
	   	../../../../2dsg/touchevent.cpp \
	   	../../../../2dsg/keyboardevent.cpp \
        ../Common/imgui_bindings.cpp 


LOCAL_MODULE           := imgui
LOCAL_ARM_MODE         := arm
LOCAL_CFLAGS           := -O2
LOCAL_C_INCLUDES       += $(addprefix $(LOCAL_PATH)/../,$(INCS))
LOCAL_SRC_FILES        := $(addprefix ../,$(LSOURCES))
LOCAL_LDLIBS           := -ldl -llog
LOCAL_SHARED_LIBRARIES := lua gideros gvfs

include $(BUILD_SHARED_LIBRARY)
