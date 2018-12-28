EMCC=$(EMSDK_PREFIX) emcc
BUILD=Build

OPTS=$(POPTS) -Os
ifneq ($(DEBUG),)
OPTS+=-g4 -s ASSERTIONS=1
endif

OBJS=$(addprefix $(BUILD)$(FLAVOUR)/,$(addsuffix .em.o,$(SRCS)))
CINCS=$(addprefix -I../,$(INCS))
CFLGS=-DFT2_BUILD_LIBRARY -DDARWIN_NO_CARBON -DHAVE_UNISTD_H \
	-DOPT_GENERIC -DREAL_IS_FLOAT \
	$(OPTS) -DFLAVOUR_$(FLAVOUR)
CFLGS+=-fno-exceptions #WASM side modules doesn't seem to support C++ exceptions

	
ifneq ($(FLAVOURS),)
allflavours: $(addsuffix .flavour,$(FLAVOURS))

%.flavour:
	@FLAVOUR=$* $(MAKE) all 
endif

all: path $(OBJS)
	@echo "EMLINK JS" $(TARGET)
	@$(EMCC) $(OBJS) -s SIDE_MODULE=1 -s WASM=0 -s DISABLE_EXCEPTION_CATCHING=0 $(OPTS) -o $(BUILD)/$(TARGET).js
	@echo "SYMGEN" $(TARGET)
	@$(EMCC) $(OBJS) -s SIDE_MODULE=1 -s WASM=1 -s DISABLE_EXCEPTION_CATCHING=0 $(OPTS) -g -o $(BUILD)/$(TARGET).wasm
	@grep 'import "env"' $(BUILD)/$(TARGET).wast | grep '(func'| cut -d' ' -f4 >$(HTML5_ROOT)/Build/$(TARGET).syms
	@echo "EMLINK WASM" $(TARGET)
	@$(EMCC) $(OBJS) -s SIDE_MODULE=1 -s WASM=1 -s DISABLE_EXCEPTION_CATCHING=0 $(OPTS) -o $(BUILD)/$(TARGET).wasm

path:
	mkdir -p  $(BUILD) $(sort $(dir $(OBJS)))

clean:
	rm -rf $(BUILD)* 

$(BUILD)$(FLAVOUR)/%.em.o: ../%.cpp
	@echo "EMC+ $<"
	@$(EMCC) $(CINCS) $(CFLGS) -std=c++11 -c $< --default-obj-ext .em.o -o $@

$(BUILD)$(FLAVOUR)/%.em.o: ../%.c
	@echo "EMCC $<"
	@$(EMCC) $(CINCS) $(CFLGS) -c $< --default-obj-ext .em.o -o $@

$(BUILD)$(FLAVOUR)/%.em.o: %.cpp
	@echo "EMC+ $<"
	@$(EMCC) $(CINCS) $(CFLGS) -std=c++11 -c $< --default-obj-ext .em.o -o $@

$(BUILD)$(FLAVOUR)/%.em.o: %.c
	@echo "EMCC $<"
	@$(EMCC) $(CINCS) $(CFLGS) -c $< --default-obj-ext .em.o -o $@
	