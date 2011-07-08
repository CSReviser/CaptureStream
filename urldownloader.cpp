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
