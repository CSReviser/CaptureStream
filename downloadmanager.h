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

#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QList>
#include <QNetworkReply>
#include <QUrl>
#include <QEventLoop>
#include <QMutex>
#include <QStringList>

class DownloadManager: public QObject {
	Q_OBJECT

public:
	DownloadManager( bool _reread, bool _past );
	void singleShot();

	QStringList flvList;
	QStringList flvListBefore20100323;

private:
	void doDownload( const QUrl &url );

	QEventLoop eventLoop;
	bool reread;
	bool past;
	mutable QMutex mutex;
	QNetworkAccessManager manager;
	QList<QNetworkReply *> currentDownloads;

public slots:
	void execute();
	void downloadFinished( QNetworkReply *reply );
};

#endif // DOWNLOADMANAGER_H
