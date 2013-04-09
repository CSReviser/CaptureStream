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

#include "urldownloader.h"

#include <QTimer>

UrlDownloader::UrlDownloader() {
	connect( &manager, SIGNAL(finished( QNetworkReply* )), SLOT(downloadFinished( QNetworkReply* )) );
}

void UrlDownloader::doDownload( const QUrl& url ) {
	this->url = url;
	QTimer::singleShot( 0, this, SLOT(execute()) );
	eventLoop.exec();
}

void UrlDownloader::execute() {
	QNetworkRequest request( url );
	manager.get( request );
}

void UrlDownloader::downloadFinished( QNetworkReply* reply ) {
	if ( !reply->error() )
		byteArray = reply->readAll();

	reply->deleteLater();
	eventLoop.exit();
}
