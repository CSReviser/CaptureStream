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
	DownloadManager( bool _reread );
	void singleShot();

	QStringList flvList;
	QStringList flvListBefore20100323;

private:
	void doDownload( const QUrl &url );

	QEventLoop eventLoop;
	bool reread;
	mutable QMutex mutex;
	QNetworkAccessManager manager;
	QList<QNetworkReply *> currentDownloads;

public slots:
	void execute();
	void downloadFinished( QNetworkReply *reply );
};

#endif // DOWNLOADMANAGER_H
