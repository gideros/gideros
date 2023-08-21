LINUX_CHAIN=$(LINUX_BIN)
LINUX_CC=$(LINUX_CHAIN)gcc
LINUX_CXX=$(LINUX_CHAIN)g++

BUILD=Build


OBJS=$(addprefix $(BUILD)/,$(addsuffix .o,$(SRCS)))
CINCS=$(addprefix -I../,$(INCS)) -I$(ROOT)/Sdk/include
CFLGS+=-DFT2_BUILD_LIBRARY -DDARWIN_NO_CARBON -DHAVE_UNISTD_H \
	-DOPT_GENERIC -DREAL_IS_FLOAT \
	-O2 -fPIC
CXXFLGS+=-std=c++11
CXXINCS+=
LFLGS+=-L$(ROOT)/Sdk/lib/linux -llua -lgvfs -lgid -lgideros

all: path $(OBJS)
	@echo "LINK" $(TARGET)
	@$(LINUX_CXX) -shared $(OBJS) $(LFLGS) -o $(BUILD)/$(TARGET).so

path:
	mkdir -p $(sort $(dir $(OBJS)))

clean:
	rm -rf $(OBJS) $(BUILD)
	
$(BUILD)/%.o: ../%.cpp
	@echo "C+ $<"
	@$(LINUX_CXX) $(CXXFLGS) $(CINCS) $(CXXINCS) $(CFLGS) -c $< -o $@

$(BUILD)/%.o: ../%.cc
	@echo "C+ $<"
	@$(LINUX_CXX) $(CXXFLGS) $(CINCS) $(CXXINCS) $(CFLGS) -c $< -o $@

$(BUILD)/%.o: ../%.c
	@echo "CC $<"
	@$(LINUX_CC) $(CINCS) $(CFLGS) -c $< -o $@

$(BUILD)/%.o: %.cpp
	@echo "C+ $<"
	@$(LINUX_CXX) $(CXXFLGS) $(CINCS) $(CXXINCS)  $(CFLGS) -c $< -o $@

$(BUILD)/%.o: %.cc
	@echo "C+ $<"
	@$(LINUX_CXX) $(CXXFLGS) $(CINCS) $(CXXINCS)  $(CFLGS) -c $< -o $@

$(BUILD)/%.o: %.c
	@echo "CC $<"
	@$(LINUX_CC) $(CINCS) $(CFLGS) -c $< -o $@
		
