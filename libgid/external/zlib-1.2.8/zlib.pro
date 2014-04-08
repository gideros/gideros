QT       -= core gui

TARGET = zlibx
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
        adler32.c \
        compress.c \
        crc32.c \
        deflate.c \
        gzclose.c \
        gzlib.c \
        gzread.c \
        gzwrite.c \
        inflate.c \
        infback.c \
        inftrees.c \
        inffast.c \
        trees.c \
        uncompr.c \
        zutil.c

