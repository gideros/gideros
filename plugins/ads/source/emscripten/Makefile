TARGET=Ads
SRCS=$(addprefix emscripten/,adsbinder ads)
INCS=$(addprefix ../../../Sdk/,include include/gideros)

build: all
	@cp ads.js $(BUILD)/Html/AdsBase.js
	@cp AdsAdSense.js $(BUILD)/Html

include ../../../../emscripten/Plugin.mk

