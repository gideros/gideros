TARGET=Ogg
INCS= ../../../Sdk/include
INCS += ../../../Sdk/include/gideros
INCS += ../../../2dsg
INCS += ../../../2dsg/gfxbackends
INCS += ../../../libgideros
INCS += ../../../libgid/include
INCS += ../../../luabinding
INCS += ../../../lua/src

XIPH_OGG=libogg-1.3.2
XIPH_THEORA=libtheora-1.1.1
XIPH_VORBIS=libvorbis-1.3.5

SOGG_F=bitwise framing
STHEORADEC_F=apiwrapper bitpack decapiwrapper decinfo decode dequant fragment huffdec idct th_info internal quant state
SVORBIS_F=mdct block window synthesis info floor1 floor0 res0 mapping0 registry codebook sharedbook envelope psy bitrate lpc lsp smallft vorbisfile
SVORBIS_F+=analysis vorbisenc

SXIPH+=$(addprefix $(XIPH_OGG)/src/,$(SOGG_F))
SXIPH+=$(addprefix $(XIPH_THEORA)/lib/,$(STHEORADEC_F))
SXIPH+=$(addprefix $(XIPH_VORBIS)/lib/,$(SVORBIS_F))

INCS += $(XIPH_OGG)/include
INCS += $(XIPH_THEORA)/include
INCS += $(XIPH_VORBIS)/include $(XIPH_VORBIS)/lib

SRCS+= Common/oggbinder \
    $(SXIPH)

include ../../../../emscripten/Plugin.mk
