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

XIPH_OGG:=libogg-1.3.2
XIPH_THEORA:=libtheora-1.1.1
XIPH_VORBIS:=libvorbis-1.3.5

SOGG_F:=bitwise framing
STHEORADEC_F:=apiwrapper bitpack decapiwrapper decinfo decode dequant fragment huffdec idct th_info internal quant state
SVORBIS_F:=mdct block window synthesis info floor1 floor0 res0 mapping0 registry codebook sharedbook envelope psy bitrate lpc lsp smallft vorbisfile

SXIPH:=$(addprefix $(XIPH_OGG)/src/,$(SOGG_F))
SXIPH+=$(addprefix $(XIPH_THEORA)/lib/,$(STHEORADEC_F))
SXIPH+=$(addprefix $(XIPH_VORBIS)/lib/,$(SVORBIS_F))

INCS += $(XIPH_OGG)/include
INCS += $(XIPH_THEORA)/include
INCS += $(XIPH_VORBIS)/include

LOCAL_MODULE           := ogg
LOCAL_ARM_MODE         := arm
LOCAL_CFLAGS           := -O2
LOCAL_C_INCLUDES       += $(addprefix $(LOCAL_PATH)/../,$(INCS))
LOCAL_SRC_FILES        := $(addprefix ../,Common/oggbinder.cpp $(addsuffix .c,$(SXIPH)))
LOCAL_LDLIBS           := -ldl -llog
LOCAL_SHARED_LIBRARIES := lua gideros gvfs

include $(BUILD_SHARED_LIBRARY)
