# -------------------------------------------------
# Project created by QtCreator 2009-04-20T14:05:07
# -------------------------------------------------
QT += network \
    xml \
	xmlpatterns \
	script
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

macx {
	#CONFIG += x86 ppc #x86_64
	#INCLUDEPATH += /Library/Frameworks/qca.framework/Versions/2/Headers
	#LIBS += -L/Library/Frameworks -framework qca

	QMAKE_CFLAGS_RELEASE += -fvisibility=hidden
	QMAKE_CXXFLAGS_RELEASE += -fvisibility=hidden

	release {
		QMAKE_POST_LINK += macdeployqt CaptureStream.app
	}
}
