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

#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include <QThread>

#include "mainwindow.h"

class DownloadThread : public QThread {
	Q_OBJECT

public:
	DownloadThread( Ui::MainWindowClass* ui );
	~DownloadThread() {}
	void cancel() { isCanceled = true; }

protected:
	void run();

signals:
	void critical( QString message );
	void information( QString message );
	void current( QString );
	void messageWithoutBreak( QString );

private:
	QStringList getAttribute( QString url, QString attribute );
	bool checkFlvstreamer( QString& path );
	bool checkOutputDir( QString dirPath );
	void id3tag( QString fullPath, QString album, QString title, QString year, QString artist );
	void downloadCharo();
	void downloadENews( bool re_read );
	bool captureStream( QString kouza, QString hdate, QString file, int retryCount, bool guess = false );
	void downloadOneWeek( int i, int addDays, QStringList& fileList, QStringList& kouzaList, QStringList& hdateList );
	QString formatName( QString format, QString kouza, QString hdate, QString file, bool checkIllegal );
	QStringList getElements( QString url, QString path );
	void downloadShower();

	Ui::MainWindowClass* ui;
	bool isCanceled;

	static QString paths[];
	static QString prefix;
	static QString suffix;

	static QString flv_host;
	static QString flv_app;
	static QString flv_service_prefix;

	static QString flvstreamer;
	static QString scramble;

};

#endif // DOWNLOADTHREAD_H
