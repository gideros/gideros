TARGET=Facebook
SRCS=$(addprefix emscripten/,facebookbinder gfacebook)
INCS=../../../Sdk/include

build: all
	@mv $(BUILD)/Facebook.js $(BUILD)/Facebook2.js
	@cat gfacebook.js $(BUILD)/Facebook2.js >$(BUILD)/Facebook.js
	@rm $(BUILD)/Facebook2.js

include ../../../../emscripten/Plugin.mk

