TARGET=imgui

INCS = ../../../Sdk/include
INCS += ../../../Sdk/include/gideros
INCS += ../../../2dsg
INCS += ../../../2dsg/gfxbackends
INCS += ../../../libgideros
INCS += ../../../libgid/include
INCS += ../../../luabinding
INCS += ../../../lua/src
INCS += Common/imgui_src
INCS += Common

SRCS= \
        Common/imgui_src/imgui \
        Common/imgui_src/imgui_demo \
        Common/imgui_src/imgui_draw \
        Common/imgui_src/imgui_widgets \
	   	../../../luabinding/binder \
        Common/imgui_bindings

#LIBS += -L"../../../Sdk/lib/desktop" -llua -lgid -lgideros -lgvfs

include $(ROOT)/scripts/GidWin32Plugin.mk
