#ifndef URLDOWNLOADER_H
#define URLDOWNLOADER_H

#include <QObject>
#include <QUrl>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QNetworkReply>

class UrlDownloader: public QObject {
	Q_OBJECT

public:
	UrlDownloader();
	void doDownload( const QUrl& url );
	const QByteArray contents() { return byteArray; }

private:
	QNetworkAccessManager manager;
	QUrl url;
	QEventLoop eventLoop;
	QByteArray byteArray;

public slots:
	void execute();
	void downloadFinished( QNetworkReply* reply );
};

#endif // URLDOWNLOADER_H
