TARGET=ogg
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

SXIPH+=$(addprefix $(XIPH_OGG)/src/,$(SOGG_F))
SXIPH+=$(addprefix $(XIPH_THEORA)/lib/,$(STHEORADEC_F))
SXIPH+=$(addprefix $(XIPH_VORBIS)/lib/,$(SVORBIS_F))

INCS += $(XIPH_OGG)/include
INCS += $(XIPH_THEORA)/include
INCS += $(XIPH_VORBIS)/include

SRCS+= Common/oggbinder \
    $(SXIPH)

EMCC=$(EMSDK_PREFIX) emcc
BUILD=Build

OPTS=-O2
ifneq ($(DEBUG),)
OPTS+=-g4 -s ASSERTIONS=1
endif

OBJS=$(addprefix $(BUILD)/,$(addsuffix .em.o,$(SRCS)))
CINCS=$(addprefix -I../,$(INCS))
CFLGS=-DFT2_BUILD_LIBRARY -DDARWIN_NO_CARBON -DHAVE_UNISTD_H \
	-DOPT_GENERIC -DREAL_IS_FLOAT \
	$(OPTS)

all: path $(OBJS)
	@echo "EMLINK" $(TARGET)
	@$(EMCC) $(OBJS) -s SIDE_MODULE=1 -s DISABLE_EXCEPTION_CATCHING=0 $(OPTS) -o $(BUILD)/$(TARGET).js

path:
	mkdir -p $(sort $(dir $(OBJS)))

clean:
	rm -rf $(BUILD)

$(BUILD)/%.em.o: ../%.cpp
	@echo "EMCC $<"
	@$(EMCC) $(CINCS) $(CFLGS) -c $< --default-obj-ext .em.o -o $@

$(BUILD)/%.em.o: ../%.c
	@echo "EMCC $<"
	@$(EMCC) $(CINCS) $(CFLGS) -c $< --default-obj-ext .em.o -o $@

