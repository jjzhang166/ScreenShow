######################################################################
# Automatically generated by qmake (3.0) ?? 1? 14 22:29:24 2017
######################################################################

QT+=network widgets gui multimedia
TEMPLATE = app
TARGET = Server
INCLUDEPATH += $$PWD/../public/

# Input
SOURCES += main.cpp \
    sock.cpp \
    manager.cpp \
    ../public/globalfunc.cpp

HEADERS += \
    sock.h \
    ../public/dts.h \
    manager.h \
    ../public/globalfunc.h

DESTDIR=$$PWD/../bin


win32: LIBS += -L$$PWD/../libs/zlib/library/ -lzlibstatic

INCLUDEPATH += $$PWD/../libs/zlib/include
DEPENDPATH += $$PWD/../libs/zlib/include

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../libs/zlib/library/zlibstatic.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../libs/zlib/library/libzlibstatic.a



win32: LIBS += -L$$PWD/../libs/ffmpeg/lib/ -lavcodec.dll -lavformat.dll -lavutil.dll

INCLUDEPATH += $$PWD/../libs/ffmpeg/include
DEPENDPATH += $$PWD/../libs/ffmpeg/include

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../libs/ffmpeg/lib/avcodec.lib \
    $$PWD/../libs/ffmpeg/lib/libavformat.dll.a \
    $$PWD/../libs/ffmpeg/lib/libavutil.dll.a
else:win32-g++: PRE_TARGETDEPS += $$PWD/../libs/ffmpeg/lib/libavcodec.dll.a \
    $$PWD/../libs/ffmpeg/lib/libavformat.dll.a \
    $$PWD/../libs/ffmpeg/lib/libavutil.dll.a
