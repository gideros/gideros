TARGET=Ads
SRCS=$(addprefix emscripten/,adsbinder ads)
INCS=$(addprefix ../../../Sdk/,include include/gideros)

build: all
	@cp ads.js $(BUILD)/AdsBase.js
	@cp AdsAdSense.js $(BUILD)

include ../../../../emscripten/Plugin.mk

