EMCC=$(EMSDK_PREFIX) emcc
BUILD=Build

LOPTS?=
OPTS=$(POPTS) -s SIDE_MODULE=2
ifneq ($(DEBUG),)
OPTS+=-gsource-map -s ASSERTIONS=1
else
OPTS+=-Os
endif

GIDSDK?=../../..

SRCS+=$(GIDSDK)/emscripten/PluginCommon
WOBJS=$(addprefix $(BUILD)$(FLAVOUR)/,$(addsuffix .emw.o,$(SRCS)))
CINCS=$(addprefix -I../,$(INCS))
CFLGS+=-DFT2_BUILD_LIBRARY -DDARWIN_NO_CARBON -DHAVE_UNISTD_H \
	-DOPT_GENERIC -DREAL_IS_FLOAT \
	$(OPTS) -DFLAVOUR_$(FLAVOUR) $(addprefix -DFLAVOUR=,$(FLAVOUR))
CFLGS+=-fno-rtti #WASM side modules doesn't seem to support C++ exceptions, and RTTI doesn't work well with DCE
CFLGS+=-DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0 -DNO_EXCEPTIONS
	
ifneq ($(FLAVOURS),)
allflavours: $(addsuffix .flavour,$(FLAVOURS))

%.flavour:
	@FLAVOUR=$* $(MAKE) all 
endif

all: path $(WOBJS)
	@echo "EMLINK WASM" $(TARGET)
	@$(EMCC) -s DISABLE_EXCEPTION_CATCHING=1 $(OPTS) $(LOPTS) -o $(BUILD)/Html/$(TARGET).wasm $(WOBJS)
	@echo "SYMGEN" $(TARGET)
	@$(EMSDK_PREFIX) wasm-dis.exe $(BUILD)/Html/$(TARGET).wasm >$(BUILD)/$(TARGET).dis
	@grep '(import ' $(BUILD)/$(TARGET).dis | grep -v '(table ' | grep -v '(memory ' | sed 's/^[ \t]*//' | cut -d' ' -f3 >$(BUILD)/$(TARGET).isyms
	@grep '(export ' $(BUILD)/$(TARGET).dis | sed 's/^[ \t]*//' | cut -d' ' -f2 >$(BUILD)/$(TARGET).esyms
	@cat $(BUILD)/*.esyms >$(BUILD)/$(TARGET).asyms
	@grep -Fvxf $(BUILD)/$(TARGET).asyms $(BUILD)/$(TARGET).isyms >$(HTML5_ROOT)/Build/$(TARGET).syms

path:
	@mkdir -p  $(BUILD) $(BUILD)/Html $(sort $(dir $(WOBJS)))

clean:
	rm -rf $(BUILD)* 

$(BUILD)$(FLAVOUR)/%.emw.o: ../%.cpp
	@echo "EMWC+ $<"
	@$(EMCC) $(CINCS) $(CFLGS) -std=c++11 -c $< -o $@

$(BUILD)$(FLAVOUR)/%.emw.o: ../%.cc
	@echo "EMWC+ $<"
	@$(EMCC) $(CINCS) $(CFLGS) -std=c++11 -c $< -o $@

$(BUILD)$(FLAVOUR)/%.emw.o: ../%.c
	@echo "EMWCC $<"
	@$(EMCC) $(CINCS) $(CFLGS) -c $< -o $@

$(BUILD)$(FLAVOUR)/%.emw.o: %.cpp
	@echo "EMWC+ $<"
	@$(EMCC) $(CINCS) $(CFLGS) -std=c++11 -c $< -o $@

$(BUILD)$(FLAVOUR)/%.emw.o: %.cc
	@echo "EMWC+ $<"
	@$(EMCC) $(CINCS) $(CFLGS) -std=c++11 -c $< -o $@

$(BUILD)$(FLAVOUR)/%.emw.o: %.c
	@echo "EMWCC $<"
	@$(EMCC) $(CINCS) $(CFLGS) -c $< -o $@
