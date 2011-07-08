/*
   Copyright (C) 2009-2011 jakago

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

#include <QtGui/QApplication>
#include "mainwindow.h"
#include <stdio.h>
#include <QDebug>

bool oneTime = false;

int main(int argc, char *argv[])
{
#if defined(QT_NO_DEBUG)
#ifdef Q_WS_WIN
	const char* null = "nul";
#else
	const char* null = "/dev/null";
#endif
	freopen( null, "a", stdout );
	freopen( null, "a", stderr );
#endif

	QApplication a(argc, argv);
	oneTime = QCoreApplication::arguments().contains( "-nogui" );
	MainWindow w;
	oneTime ? w.download() : w.show();
	return a.exec();
}
