TARGET=harfbuzz

INCS += ../../../Sdk/include
INCS += ../../../Sdk/include/gideros
INCS += ../../../2dsg
INCS += ../../../2dsg/gfxbackends
INCS += ../../../libgideros
INCS += ../../../libgid/include
INCS += ../../../luabinding
INCS += ../../../$(LUA_INCLUDE)
HB=../harfbuzz/src
INCS += $(HB) $(HB)/hb-ucdn

SRCS += \
		$(basename Common/harfbuzzbinder.cpp \
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
		$(addprefix $(HB)/hb-ot-shape-complex-,arabic hangul hebrew default indic-table indic khmer myanmar thai use-table use vowel-constraints) \
  		$(HB)/hb-ucdn.cc \
  		$(HB)/hb-ucdn/ucdn.c)

include $(ROOT)/scripts/GidWin32Plugin.mk
CFLGS+=-DHAVE_UCDN
