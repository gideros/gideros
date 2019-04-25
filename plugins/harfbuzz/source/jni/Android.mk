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

####

include $(CLEAR_VARS)

INCS += ../../../Sdk/include
INCS += ../../../Sdk/include/gideros
INCS += ../../../2dsg
INCS += ../../../2dsg/gfxbackends
INCS += ../../../libgideros
INCS += ../../../libgid/include
INCS += ../../../luabinding
INCS += ../../../lua/src
HB=../harfbuzz/src
INCS += $(HB) $(HB)/hb-ucdn

LOCAL_MODULE           := harfbuzz
LOCAL_ARM_MODE         := arm
LOCAL_CFLAGS           := -O2
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/../../../../Sdk/include $(addprefix $(LOCAL_PATH)/../,$(INCS))
LOCAL_SRC_FILES        := 
LOCAL_LDLIBS           := -ldl -llog
LOCAL_SHARED_LIBRARIES := lua gideros

LOCAL_SRC_FILES += $(addprefix ../,\
		Common/harfbuzzbinder.cpp \
		$(HB)/hb-common.cc \
		$(HB)/hb-cst.cc $(HB)/hb-warning.cc \
		$(HB)/hb-shape.cc $(HB)/hb-shape-plan.cc $(HB)/hb-shaper.cc\
		$(HB)/hb-font.cc $(HB)/hb-face.cc \
		$(HB)/hb-blob.cc $(HB)/hb-map.cc $(HB)/hb-set.cc\
		$(HB)/hb-buffer.cc $(HB)/hb-buffer-serialize.cc $(HB)/hb-unicode.cc\
  		$(HB)/hb-aat-map.cc $(HB)/hb-aat-layout.cc\
  		$(HB)/hb-ot-cff1-table.cc $(HB)/hb-ot-cff2-table.cc $(HB)/hb-ot-color.cc $(HB)/hb-ot-face.cc $(HB)/hb-ot-font.cc \
  		$(HB)/hb-ot-layout.cc $(HB)/hb-ot-map.cc $(HB)/hb-ot-math.cc $(HB)/hb-ot-name.cc $(HB)/hb-ot-name-language.cc \
  		$(HB)/hb-ot-shape.cc $(HB)/hb-ot-tag.cc $(HB)/hb-ot-var.cc $(HB)/hb-ot-shape-fallback.cc $(HB)/hb-ot-shape-normalize.cc \
		$(addsuffix .cc,$(addprefix $(HB)/hb-ot-shape-complex-,arabic hangul hebrew default indic-table indic khmer myanmar thai use-table use vowel-constraints)) \
  		$(HB)/hb-ucdn.cc \
  		$(HB)/hb-ucdn/ucdn.c)

LOCAL_CFLAGS += -DHAVE_UCDN

include $(BUILD_SHARED_LIBRARY)

