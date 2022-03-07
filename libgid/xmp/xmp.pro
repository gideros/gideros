CONFIG -= qt

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++17

defineReplace(expand) {
    variable = $$1
    prefix=$$2
    suffix=$$3
    names = $$eval($$variable)
    expanded =

    for(name, names) {
        expanded+= $${prefix}$${name}$${suffix}
    }
    return ($$expanded)
}
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES+=_REENTRANT LIBXMP_CORE_PLAYER
XMP_SRC=virtual period player read_event dataio lfo envelope \
                scan control filter effects mixer mix_all load_helpers load \
                hio smix memio
XMP_HDR=common effects envelope format lfo list mixer period player \
                virtual precomp_lut hio memio mdataio tempfile
XMP_LOADERS=xm_load s3m_load it_load \
                        common itsex sample
XMP_LOADERS_HDR=it loader mod s3m xm
SOURCES += $$expand(XMP_SRC,../external/libxmp-4.3/src/,.c)
SOURCES += $$expand(XMP_LOADERS,../external/libxmp-4.3/src/loaders/,.c)
SOURCES += \
        ../external/libxmp-4.3/lite/src/format.c \
        ../external/libxmp-4.3/lite/src/loaders/mod_load.c
HEADERS += $$expand(XMP_HDR,../external/libxmp-4.3/src/,.h)
HEADERS += $$expand(XMP_LOADERS_HDR,../external/libxmp-4.3/src/loaders/,.h)
INCLUDEPATH += "../external/libxmp-4.3/include"
INCLUDEPATH += "../external/libxmp-4.3/src"
INCLUDEPATH += "../external/libxmp-4.3/src/loaders"
INCLUDEPATH += ../../libgvfs

