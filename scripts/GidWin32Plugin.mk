WIN32_CHAIN=PATH=$(WIN32_BIN):$(PATH) $(WIN32_BIN)/
WIN32_CC=$(WIN32_CHAIN)gcc
WIN32_CXX=$(WIN32_CHAIN)g++

BUILD=Build


OBJS=$(addprefix $(BUILD)/,$(addsuffix .o,$(SRCS)))
CINCS=$(addprefix -I../,$(INCS)) -I$(ROOT)/Sdk/include
CFLGS+=-DFT2_BUILD_LIBRARY -DDARWIN_NO_CARBON -DHAVE_UNISTD_H \
	-DOPT_GENERIC -DREAL_IS_FLOAT \
	-O2 -DUNICODE_ -municode
CXXFLGS+=-std=c++11
CXXINCS+=
LFLGS+=-L$(ROOT)/Sdk/lib/win32 -llua -lgvfs -lgid -lgideros

all: path $(OBJS)
	@echo "LINK" $(TARGET)
	@$(WIN32_CXX) -shared $(OBJS) $(LFLGS) -o $(BUILD)/$(TARGET).dll

path:
	mkdir -p $(sort $(dir $(OBJS)))

clean:
	rm -rf $(OBJS) $(BUILD)
	
$(BUILD)/%.o: ../%.cpp
	@echo "C+ $<"
	@$(WIN32_CXX) $(CXXFLGS) $(CINCS) $(CXXINCS) $(CFLGS) -c $< -o $@

$(BUILD)/%.o: ../%.cc
	@echo "C+ $<"
	@$(WIN32_CXX) $(CXXFLGS) $(CINCS) $(CXXINCS) $(CFLGS) -c $< -o $@

$(BUILD)/%.o: ../%.c
	@echo "CC $<"
	@$(WIN32_CC) $(CINCS) $(CFLGS) -c $< -o $@

$(BUILD)/%.o: %.cpp
	@echo "C+ $<"
	@$(WIN32_CXX) $(CXXFLGS) $(CINCS) $(CXXINCS)  $(CFLGS) -c $< -o $@

$(BUILD)/%.o: %.cc
	@echo "C+ $<"
	@$(WIN32_CXX) $(CXXFLGS) $(CINCS) $(CXXINCS)  $(CFLGS) -c $< -o $@

$(BUILD)/%.o: %.c
	@echo "CC $<"
	@$(WIN32_CC) $(CINCS) $(CFLGS) -c $< -o $@
		