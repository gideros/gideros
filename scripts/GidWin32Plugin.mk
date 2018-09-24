
CC=gcc
CX=g++
BUILD=Build


OBJS=$(addprefix $(BUILD)/,$(addsuffix .o,$(SRCS)))
CINCS=$(addprefix -I../,$(INCS)) -I$(ROOT)/Sdk/include
CFLGS+=-DFT2_BUILD_LIBRARY -DDARWIN_NO_CARBON -DHAVE_UNISTD_H \
	-DOPT_GENERIC -DREAL_IS_FLOAT \
	-O2
CXXFLGS+=-std=c++11
CXXINCS+=
LFLGS+=-L$(ROOT)/Sdk/lib/win32 -llua -lgvfs -lgid -lgideros

all: path $(OBJS)
	@echo "LINK" $(TARGET)
	@$(CX) -shared $(OBJS) $(LFLGS) -o $(BUILD)/$(TARGET).dll

path:
	mkdir -p $(sort $(dir $(OBJS)))

clean:
	rm -rf $(BUILD)
	
$(BUILD)/%.o: ../%.cpp
	@echo "C+ $<"
	@$(CX) $(CXXFLGS) $(CINCS) $(CXXINCS) $(CFLGS) -c $< -o $@

$(BUILD)/%.o: ../%.c
	@echo "CC $<"
	@$(CC) $(CINCS) $(CFLGS) -c $< -o $@

$(BUILD)/%.o: %.cpp
	@echo "C+ $<"
	@$(CX) $(CXXFLGS) $(CINCS) $(CXXINCS)  $(CFLGS) -c $< -o $@

$(BUILD)/%.o: %.c
	@echo "CC $<"
	@$(CC) $(CINCS) $(CFLGS) -c $< -o $@
		