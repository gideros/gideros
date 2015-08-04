QT -= core gui

TARGET = mpg123
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    compat.c \
    dct64.c \
    dither.c \
    equalizer.c \
    feature.c \
    format.c \
    frame.c \
    icy.c \
    icy2utf8.c \
    id3.c \
    index.c \
    layer1.c \
    layer2.c \
    layer3.c \
    libmpg123.c \
    ntom.c \
    optimize.c \
    parse.c \
    readers.c \
    stringbuf.c \
    synth.c \
    synth_8bit.c \
    synth_real.c \
    synth_s32.c \
    tabinit.c

INCLUDEPATH += .. .

DEFINES += OPT_GENERIC REAL_IS_FLOAT
