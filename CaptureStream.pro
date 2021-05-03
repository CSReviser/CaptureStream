# -------------------------------------------------
# Project created by QtCreator 2009-04-20T14:05:07
# -------------------------------------------------
QT += network \
	xml \
	xmlpatterns \
	script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
	mp3.cpp \
	urldownloader.cpp
HEADERS += mainwindow.h \
	downloadthread.h \
	downloadmanager.h \
	customizedialog.h \
	messagewindow.h \
	scrambledialog.h \
	utility.h \
	mp3.h \
	urldownloader.h \
	qt4qt5.h
FORMS += mainwindow.ui \
	customizedialog.ui \
	messagewindow.ui \
	scrambledialog.ui

linux {
	QMAKE_LFLAGS += -no-pie
}

macx {
	QMAKE_CFLAGS_RELEASE += -fvisibility=hidden
	QMAKE_CXXFLAGS_RELEASE += -fvisibility=hidden

	x86 {
		CONFIG += ppc
		QMAKE_CC = gcc-4.0
		QMAKE_CXX = g++-4.0
		QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk
		QMAKE_CFLAGS_RELEASE += -mmacosx-version-min=10.4
		QMAKE_CXXFLAGS_RELEASE += -mmacosx-version-min=10.4
	}

	release {
		QMAKE_POST_LINK += macdeployqt CaptureStream.app
	}
}

OTHER_FILES += \
	stylesheet-mac.qss \
	stylesheet-win.qss \
	stylesheet-ubu.qss

RESOURCES += \
    stylesheet.qrc
