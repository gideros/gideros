EMCC=$(EMSDK_PREFIX) emcc
BUILD=Build

OPTS=$(POPTS) -Os 
ifneq ($(DEBUG),)
OPTS+=-g4 -s ASSERTIONS=1 --source-map-base http://hieroglyphe.net/gideros/BugTest/
endif

WOBJS=$(addprefix $(BUILD)$(FLAVOUR)/,$(addsuffix .emw.o,$(SRCS)))
CINCS=$(addprefix -I../,$(INCS))
CFLGS+=-DFT2_BUILD_LIBRARY -DDARWIN_NO_CARBON -DHAVE_UNISTD_H \
	-DOPT_GENERIC -DREAL_IS_FLOAT \
	$(OPTS) -DFLAVOUR_$(FLAVOUR) $(addprefix -DFLAVOUR=,$(FLAVOUR))
CFLGS+=-fno-exceptions -fno-rtti #WASM side modules doesn't seem to support C++ exceptions, and RTTI doesn't work well with DCE
CFLGS+=-DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0
	
ifneq ($(FLAVOURS),)
allflavours: $(addsuffix .flavour,$(FLAVOURS))

%.flavour:
	@FLAVOUR=$* $(MAKE) all 
endif

all: path $(WOBJS)
	@echo "EMLINK WASM" $(TARGET)
	@$(EMCC) -s SIDE_MODULE=2 -s WASM=1 -s DISABLE_EXCEPTION_CATCHING=0 $(OPTS) -o $(BUILD)/Html/$(TARGET).wasm $(WOBJS) #-s 'BINARYEN_TRAP_MODE="clamp"'
	@echo "SYMGEN" $(TARGET)
	@$(EMSDK_PREFIX) wasm2wat.exe $(BUILD)/Html/$(TARGET).wasm | grep 'import "env"' | grep '(func' | sed 's/^[ \t]*//' | cut -d' ' -f3 >$(HTML5_ROOT)/Build/$(TARGET).syms

path:
	@mkdir -p  $(BUILD) $(BUILD)/Html $(sort $(dir $(WOBJS)))

clean:
	rm -rf $(BUILD)* 

$(BUILD)$(FLAVOUR)/%.emw.o: ../%.cpp
	@echo "EMWC+ $<"
	@$(EMCC) $(CINCS) $(CFLGS) -s WASM=1 -std=c++11 -c $< --default-obj-ext .emw.o -o $@

$(BUILD)$(FLAVOUR)/%.emw.o: ../%.cc
	@echo "EMWC+ $<"
	@$(EMCC) $(CINCS) $(CFLGS) -s WASM=1 -std=c++11 -c $< --default-obj-ext .emw.o -o $@

$(BUILD)$(FLAVOUR)/%.emw.o: ../%.c
	@echo "EMWCC $<"
	@$(EMCC) $(CINCS) $(CFLGS) -s WASM=1 -c $< --default-obj-ext .emw.o -o $@

$(BUILD)$(FLAVOUR)/%.emw.o: %.cpp
	@echo "EMWC+ $<"
	@$(EMCC) $(CINCS) $(CFLGS) -s WASM=1 -std=c++11 -c $< --default-obj-ext .emw.o -o $@

$(BUILD)$(FLAVOUR)/%.emw.o: %.cc
	@echo "EMWC+ $<"
	@$(EMCC) $(CINCS) $(CFLGS) -s WASM=1 -std=c++11 -c $< --default-obj-ext .emw.o -o $@

$(BUILD)$(FLAVOUR)/%.emw.o: %.c
	@echo "EMWCC $<"
	@$(EMCC) $(CINCS) $(CFLGS) -s WASM=1 -c $< --default-obj-ext .emw.o -o $@
