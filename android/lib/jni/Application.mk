APP_STL := c++_static
APP_CPPFLAGS += -fexceptions -frtti
APP_PLATFORM := android-18
#APP_ABI := all
ifneq ($(OCULUS),)
APP_ABI := arm64-v8a armeabi-v7a
endif
APP_OPTIM := release
APP_UNIFIED_HEADERS := true

