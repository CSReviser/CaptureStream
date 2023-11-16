# -------------------------------------------------
# Project created by QtCreator 2009-04-20T14:05:07
# -------------------------------------------------
QT += network \
	gui \
	core \
	xml \
	widgets
#	xmlpatterns \
#	script

equals(QT_MAJOR_VERSION, 5){
   QT += xmlpatterns
   QT += script
}

lessThan(QT_MAJOR_VERSION, 6): QT += xmlpatterns
lessThan(QT_MAJOR_VERSION, 6): QT += script
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat
#equals(QT_MAJOR_VERSION, 6):  QT += core5compat
DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x050F00

TARGET = CaptureStream
TEMPLATE = app
SOURCES += main.cpp \
	mainwindow.cpp \
	downloadthread.cpp \
	downloadmanager.cpp \
	customizedialog.cpp \
	messagewindow.cpp \
	scrambledialog.cpp \
	utility.cpp \
#	mp3.cpp \
	urldownloader.cpp
equals(QT_MAJOR_VERSION, 5):  SOURCES += mp3.cpp
HEADERS += mainwindow.h \
	downloadthread.h \
	downloadmanager.h \
	customizedialog.h \
	messagewindow.h \
	scrambledialog.h \
	utility.h \
#	mp3.h \
	urldownloader.h \
	qt4qt5.h
equals(QT_MAJOR_VERSION, 5):  HEADERS += mp3.h
FORMS += mainwindow.ui \
	customizedialog.ui \
	messagewindow.ui \
	scrambledialog.ui

windows {
	RC_ICONS = icon.ico
}

linux {
	QMAKE_LFLAGS += -no-pie
	LIBS +=-lrt
	CONFIG += static
}

macx {
	QMAKE_CFLAGS_RELEASE += -fvisibility=hidden
	QMAKE_CXXFLAGS_RELEASE += -fvisibility=hidden
#	QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
	
	x86 {
#		CONFIG += ppc
#		QMAKE_CC = gcc-4.0
#		QMAKE_CXX = g++-4.0
#		QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.12u.sdk
		QMAKE_CFLAGS_RELEASE += -mmacosx-version-min=10.12
		QMAKE_CXXFLAGS_RELEASE += -mmacosx-version-min=10.12
		ICON = icon.icns
	}

	release {
		QMAKE_POST_LINK += macdeployqt CaptureStream.app
	}
}

OTHER_FILES += \
	stylesheet-mac.qss \
	stylesheet-win.qss \
	stylesheet-ubu.qss \
	icon.png

RESOURCES += \
    stylesheet.qrc
