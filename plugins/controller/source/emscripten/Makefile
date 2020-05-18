TARGET=controller
SRCS=controller controllerbinder gamepad/Gamepad_private gcontroller gamepad/Gamepad_html5
INCS=../../../Sdk/include . gamepad ../../../emscripten 
CFLGS+=-DWIN32_NOQT

build: all
	@cp GiderosController.js $(BUILD)/Html

include ../../../../emscripten/Plugin.mk
