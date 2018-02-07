TARGET=Ads
SRCS=$(addprefix emscripten/,adsbinder ads)
INCS=../../../Sdk/include

build: all
	@mv $(BUILD)/Ads.js $(BUILD)/Ads2.js
	@cat ads.js $(BUILD)/Ads2.js >$(BUILD)/Ads.js
	@rm $(BUILD)/Ads2.js
	@cp AdsAdSense.js $(BUILD)

include ../../../../emscripten/Plugin.mk

