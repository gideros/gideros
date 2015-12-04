TARGET=Facebook
SRCS=$(addprefix emscripten/,facebookbinder gfacebook)
INCS=../../../Sdk/include

EMCC=emcc
BUILD=Build

OPTS=-O2 
ifneq ($(DEBUG),)
OPTS+=-g4 -s ASSERTIONS=1
endif

OBJS=$(addprefix $(BUILD)/,$(addsuffix .em.o,$(SRCS)))
CINCS=$(addprefix -I../,$(INCS))
CFLGS=-DFT2_BUILD_LIBRARY -DDARWIN_NO_CARBON -DHAVE_UNISTD_H \
	-DOPT_GENERIC -DREAL_IS_FLOAT \
	$(OPTS) --bind

all: path $(OBJS)
	emcc $(OBJS) -s SIDE_MODULE=1 -s DISABLE_EXCEPTION_CATCHING=0 $(OPTS) --pre-js gfacebook.js -o $(BUILD)/$(TARGET).js

path:
	mkdir -p $(sort $(dir $(OBJS)))

clean:
	rm -rf $(BUILD)

$(BUILD)/%.em.o: ../%.cpp
	$(EMCC) $(CINCS) $(CFLGS) -c $< --default-obj-ext .em.o -o $@

$(BUILD)/%.em.o: ../%.c
	$(EMCC) $(CINCS) $(CFLGS) -c $< --default-obj-ext .em.o -o $@

