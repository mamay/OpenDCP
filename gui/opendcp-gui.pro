#-------------------------------------------------
#
# Project created by QtCreator 2011-03-08T19:26:11
#
#-------------------------------------------------

QT       += core gui

TARGET =   OpenDCP
TEMPLATE = app

#RC_FILE = resources/opendcp.rc
ICON =  resources/opendcp.icns

SOURCES += main.cpp\
           mainwindow.cpp \
           j2k.cpp \
           mxf.cpp \
           xml.cpp \
           generatetitle.cpp

HEADERS  += mainwindow.h \
            generatetitle.h

FORMS    += mainwindow.ui \
            generatetitle.ui

#QMAKE_LFLAGS = -Wl,-subsystem,windows

WIN32_LIBS +=   /home/tmeiczin/Development/OpenDCP/build-win32/lib/libopendcp.a \
                /home/tmeiczin/Development/OpenDCP/build-win32/contrib/lib/libopenjpeg.a \
                /home/tmeiczin/Development/OpenDCP/build-win32/contrib/lib/libtiff.a \
                /home/tmeiczin/Development/OpenDCP/build-win32/contrib/lib/libasdcp.a \
                /home/tmeiczin/Development/OpenDCP/build-win32/contrib/lib/libkumu.a \
                /home/tmeiczin/Development/OpenDCP/build-win32/contrib/lib/libcrypto.a \
                /home/tmeiczin/Development/OpenDCP/build-win32/contrib/lib/libexpat.a \
                /home/tmeiczin/Development/OpenDCP/build-win32/contrib/lib/libssl.a \
                -lz

LIBS +=         -L/Users/tmeiczin/Development/OpenDCP/build/lib -lopendcp \
                -L/Users/tmeiczin/Development/OpenDCP/build/contrib/lib/ \
                -lopenjpeg -ltiff -lxmlsec1 -lxmlsec1-openssl \
                -lxslt -lxml2 -lasdcp -lkumu -lexpat -lcrypto -lssl -liconv \
                -lz -lgomp

INCLUDEPATH =   /usr/local/include /Users/tmeiczin/Development/OpenDCP/build/contrib/include

RESOURCES += \
    resources/opendcp.qrc
