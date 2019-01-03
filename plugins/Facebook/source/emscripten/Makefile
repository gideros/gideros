TARGET=Facebook
SRCS=$(addprefix emscripten/,facebookbinder gfacebook)
INCS=$(addprefix ../../../Sdk/,include include/gideros)

build: all
	@cp gfacebook.js $(BUILD)/FacebookBase.js

include ../../../../emscripten/Plugin.mk

