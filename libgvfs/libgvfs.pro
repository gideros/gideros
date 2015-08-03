QT += 

TARGET = gvfs
TEMPLATE = lib

DEFINES += GIDEROS_LIBRARY

unix {
  DEFINES += STRICT_LINUX
}

INCLUDEPATH += . ./private

HEADERS += gexport.h

HEADERS += \
    wcio.h \
    local.h \
    glue.h \
    fvwrite.h \
    floatio.h \
    fileext.h \
    defines.h \
    gstdio.h \
    gfile.h \
    gfile_p.h \
    gpath.h \
    gpath_p.h

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
    clrerr.c \
    gfile.cpp \
    gpath.cpp

LIBS += -lpthread

