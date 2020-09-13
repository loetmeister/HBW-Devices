CXX = g++
CXXFLAGS = -Wall -std=c++11 -g3 -D_DEBUG_ -D_WIN32 -DCONTROLLER_ID=AR8_ID -D__DUMMY__ -DF_CPU=32000000 -DMAPPED_EEPROM_START=0
LDFLAGS = -static

SHELL := cmd.exe
RM := del /S /Q
REL_SRC_DIR := ../../

PROJ := DeviceTests
BUILD_DIR := bin/
OBJ_EXT := .o
DEP_EXT := .d
LIB_EXT := .a
EXEC_EXT := .exe

USER_OBJS :=
LIBS := 

C_SRCS := 
OBJS := 
C_DEPS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

INCLUDE = \
-I $(REL_SRC_DIR)Firmware/HmwUnits \
-I $(REL_SRC_DIR)Firmware/Framework \
-I $(REL_SRC_DIR)Firmware/Win32 \
-I $(REL_SRC_DIR)Firmware \
-I $(REL_SRC_DIR)Tests

CPP_SRCS = \
Tests/DeviceTests/MainDeviceTests.cpp 


OBJS += $(CPP_SRCS:%.cpp=$(BUILD_DIR)%$(OBJ_EXT))
DEPS += $(OBJS:$(OBJ_EXT)=$(DEP_EXT))
EXEC = $(PROJ)$(EXEC_EXT)

SUB_DIRS := $(foreach DIR,$(sort $(dir $(OBJS))),$(DIR).)

all: $(BUILD_DIR)$(EXEC)

$(BUILD_DIR)$(EXEC): $(SUB_DIRS) $(OBJS)
	@echo linking $@
	@$(CXX) $(LDFLAGS) -o $@ $(OBJS) -Llib $(LIBS)

.PRECIOUS: %/.

# the next target creates all needed subdirs
%/.:
	mkdir "$(@:%/.=%)"

define define_obj_target
$(1:%.cpp=$(BUILD_DIR)%.o) : Makefile $(REL_SRC_DIR)$(1)
	@echo compile $(1)
	@$(CXX) $(CXXFLAGS) $(INCLUDE) -c -MD -MP -MF "$(1:%.cpp=$(BUILD_DIR)%.d)" -MT"$(1:%.cpp=$(BUILD_DIR)%.d)" -MT"$(1:%.cpp=$(BUILD_DIR)%.o)" -o "$(1:%.cpp=$(BUILD_DIR)%.o)" "$(REL_SRC_DIR)$(1)" 
endef

# generate the targets by using the above template
$(foreach SRC,$(CPP_SRCS),$(eval $(call define_obj_target,$(SRC))))

.PHONY: clean 
clean:
	$(RM) *$(EXEC_EXT) *$(LIB_EXT) *$(OBJ_EXT) *$(DEP_EXT)