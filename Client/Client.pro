######################################################################
# Automatically generated by qmake (3.0) ?? 1? 14 22:28:56 2017
######################################################################
QT += gui widgets network
TEMPLATE = app
TARGET = Client
INCLUDEPATH += $$PWD/../public

# Input
SOURCES += main.cpp \
    picshow.cpp \
    sock.cpp \
    ../public/globalfunc.cpp

FORMS += \
    picshow.ui

HEADERS += \
    picshow.h \
    sock.h \
    ../public/dts.h \
    ../public/globalfunc.h

DESTDIR=$$PWD/../bin

win32: LIBS += -L$$PWD/../libs/zlib/library/ -lzlibstatic

INCLUDEPATH += $$PWD/../libs/zlib/include
DEPENDPATH += $$PWD/../libs/zlib/include

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../libs/zlib/library/zlibstatic.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../libs/zlib/library/libzlibstatic.a

