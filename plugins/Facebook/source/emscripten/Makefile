TARGET=Facebook
SRCS=$(addprefix emscripten/,facebookbinder gfacebook)
INCS=$(addprefix ../../../Sdk/,include include/gideros)

build: all
	@cp gfacebook.js $(BUILD)/Html/FacebookBase.js

include ../../../../emscripten/Plugin.mk

