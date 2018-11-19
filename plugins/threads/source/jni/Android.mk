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

LOCAL_MODULE           := threads
LOCAL_ARM_MODE         := arm
LOCAL_CFLAGS           := -O2 -DLUA_NOCOMPAT_MODULE
LOCAL_C_INCLUDES       += ../../../Sdk/include \
                          ../../../lua/src \
                          ../../../libgid/external/zlib-1.2.8
LOCAL_SRC_FILES        := ../threads_entry.cpp \
                          ../luathread.cpp \
                          ../StateToState.cpp \
                          ../threadtimedluahook.cpp \
                          ../../../lfs/source/lfs.c \
                          ../../../../luabinding/binder.cpp \
                          ../../../../luabinding/zlibbinder.cpp \
                          ../../../luasocket/source/auxiliar.c \
                          ../../../luasocket/source/buffer.c \
                          ../../../luasocket/source/except.c \
                          ../../../luasocket/source/inet.c \
                          ../../../luasocket/source/io.c \
                          ../../../luasocket/source/luasocket.c \
                          ../../../luasocket/source/options.c \
                          ../../../luasocket/source/select.c \
                          ../../../luasocket/source/tcp.c \
                          ../../../luasocket/source/timeout.c \
                          ../../../luasocket/source/udp.c \
                          ../../../luasocket/source/mime.c \
                          ../../../luasocket/source/usocket.c \
                          ../../../json/source/strbuf.c \
                          ../../../json/source/fpconv.c \
                          ../../../json/source/lua_cjson.c
LOCAL_LDLIBS           := -ldl -llog
LOCAL_SHARED_LIBRARIES := lua gideros gvfs

include $(BUILD_SHARED_LIBRARY)
