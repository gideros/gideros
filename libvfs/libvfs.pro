QT += 

TARGET = vfs
TEMPLATE = lib

DEFINES += VFS_LIBRARY

INCLUDEPATH += . ./private

HEADERS += \
    wcio.h \
    local.h \
    glue.h \
    fvwrite.h \
    floatio.h \
    fileext.h \
    defines.h \
    gstdio.h

SOURCES += \
    wsetup.c \
    wbuf.c \
    vfscanf.c \
    vfprintf.c \
    ungetc.c \
    tmpfile.c \
    stdio.c \
    setvbuf.c \
    rget.c \
    refill.c \
    putc.c \
    makebuf.c \
    getc.c \
    fwrite.c \
    fwalk.c \
    fvwrite.c \
    ftell.c \
    fseek.c \
    fscanf.c \
    freopen.c \
    fread.c \
    fputs.c \
    fputc.c \
    fprintf.c \
    fopen.c \
    flockfile.c \
    flags.c \
    findfp.c \
    fileops.c \
    fgets.c \
    fgetc.c \
    fflush.c \
    ferror.c \
    feof.c \
    fclose.c \
    extra.c \
    clrerr.c

LIBS += -lpthread

