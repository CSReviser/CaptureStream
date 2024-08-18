/*
	Copyright (C) 2009-2013 jakago

	This file is part of CaptureStream, the flv downloader for NHK radio
	language courses.

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "utility.h"
#include "qt4qt5.h"

#include <QApplication>
#include <stdio.h>

int main(int argc, char *argv[])
{
	qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
	qputenv("QT_SCALE_FACTOR", "1");
#if defined(QT_NO_DEBUG)
#ifdef QT4_QT5_WIN
	const char* null = "nul";
#else
	const char* null = "/dev/null";
#endif
	freopen( null, "a", stdout );
	freopen( null, "a", stderr );
#endif

	QApplication a(argc, argv);
	MainWindow w;
	QGuiApplication::setWindowIcon(QIcon(":icon.png"));
	Utility::nogui() ? w.download() : w.show();
	return a.exec();
}
