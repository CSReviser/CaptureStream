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
    urldownloader.h
FORMS += mainwindow.ui \
    customizedialog.ui \
    messagewindow.ui \
    scrambledialog.ui

macx {
	#CONFIG += x86 #x86_64 ppc
	QMAKE_CFLAGS_RELEASE += -fvisibility=hidden
	QMAKE_CXXFLAGS_RELEASE += -fvisibility=hidden

	debug {
	}

	release_fw {
		QMAKE_POST_LINK += /usr/bin/install_name_tool \
			-change \
			QtCore.framework/Versions/4/QtCore \
			@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore \
			CaptureStream.app/Contents/MacOs/CaptureStream;
		QMAKE_POST_LINK += /usr/bin/install_name_tool \
			-change \
			QtNetwork.framework/Versions/4/QtNetwork \
			@executable_path/../Frameworks/QtNetwork.framework/Versions/4/QtNetwork \
			CaptureStream.app/Contents/MacOs/CaptureStream;
		QMAKE_POST_LINK += /usr/bin/install_name_tool \
			-change \
			QtXmlPatterns.framework/Versions/4/QtXmlPatterns \
			@executable_path/../Frameworks/QtXmlPatterns.framework/Versions/4/QtXmlPatterns \
			CaptureStream.app/Contents/MacOs/CaptureStream;
		QMAKE_POST_LINK += /usr/bin/install_name_tool \
			-change \
			QtGui.framework/Versions/4/QtGui \
			@executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui \
			CaptureStream.app/Contents/MacOs/CaptureStream;
		QMAKE_POST_LINK += /usr/bin/install_name_tool \
			-change \
			QtXml.framework/Versions/4/QtXml \
			@executable_path/../Frameworks/QtXml.framework/Versions/4/QtXml \
			CaptureStream.app/Contents/MacOs/CaptureStream;
		QMAKE_POST_LINK += /usr/bin/install_name_tool \
			-change \
			QtScript.framework/Versions/4/QtScript \
			@executable_path/../Frameworks/QtScript.framework/Versions/4/QtScript \
			CaptureStream.app/Contents/MacOs/CaptureStream;
		QMAKE_POST_LINK += /usr/bin/strip \
			CaptureStream.app/Contents/MacOs/CaptureStream
	}
}
