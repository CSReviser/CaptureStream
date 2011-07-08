#include "downloadmanager.h"

#include <QFileInfo>
#include <QStringList>
#include <QDate>
#include <QThread>
#include <QRegExp>
#include <QtDebug>
#include <QTimer>
#include <QMutexLocker>

#define SEARCH_20100323 "http://www.google.co.jp/search?q=video_player_wide.swf+site:cgi2.nhk.or.jp&hl=ja&lr=lang_ja&num=100&filter=0&start=0"
#define SEARCH_20090330 "http://www.google.co.jp/search?q=video_player.swf+site:cgi2.nhk.or.jp&hl=ja&lr=lang_ja&num=100&filter=0&start=0"
#define SEARCH_AT_ONCE 100
#define VIDEO_PLAYER_WIDE "video_player_wide.swf"

DownloadManager::DownloadManager( bool _reread ) : reread(_reread) {
	connect( &manager, SIGNAL(finished(QNetworkReply*)), SLOT(downloadFinished(QNetworkReply*)) );
}

void DownloadManager::singleShot() {
	QTimer::singleShot( 0, this, SLOT(execute()) );
	eventLoop.exec();
}

void DownloadManager::doDownload( const QUrl &url ) {
	QNetworkRequest request( url );
	QNetworkReply *reply = manager.get( request );

	QMutexLocker locker( &mutex );
	currentDownloads.append( reply );
}

void DownloadManager::execute() {
	QDate today = QDate::currentDate();
	QDate from( 2009, 3, 30 );	//ダウンロード可能な一番古い日付

	if ( !reread ) {
		from = today.addDays( -7 );
		doDownload( QUrl(SEARCH_20100323 ) );
		doDownload( QUrl(SEARCH_20090330 ) );
	}

	for ( QDate i = today; i >= from; i = i.addDays( -1 ) ) {
		if ( i.dayOfWeek() >= 1 && i.dayOfWeek() <= 5 ) {
			QUrl url( "http://cgi2.nhk.or.jp/e-news/news/index.cgi?ymd=" + i.toString( "yyyyMMdd" ) );
			doDownload( url );
		}
	}
}

void DownloadManager::downloadFinished( QNetworkReply *reply ) {
	QMutexLocker locker( &mutex );

	QUrl url = reply->url();
	if (reply->error()) {
		//emit critical( QString::fromUtf8( "ページ(" ) + url.toEncoded().constData() +
					   //QString::fromUtf8( ")を取得できませんでした: " ) + qPrintable( reply->errorString() ) );
	} else {
		QString urlStr = url.toString();
		if ( !reread && urlStr.startsWith( "http://www.google.co.jp/" ) ) {
			QString page = QString::fromUtf8( reply->readAll().constData() );
			//qDebug() << page;
			//QString rx = "http://cgi2.nhk.or.jp/e-news/swfp/video_player.swf.type=real&m_name=([^\"]*)";
			QString rx = "http://cgi2.nhk.or.jp/e-news/swfp/video_player(?:_wide)?.swf.type=real&amp;m_name=([^\"]*)";
			QRegExp regexp( rx, Qt::CaseInsensitive );
			QList<QString> tempList;
			int pos = 0;
			while ( ( pos = regexp.indexIn( page, pos ) ) != -1) {
				if ( !flvList.contains( regexp.cap( 1 ) ) )
					tempList << regexp.cap( 1 );
				pos += regexp.matchedLength();
			}
			QRegExp newPlayer( VIDEO_PLAYER_WIDE );
			if ( newPlayer.indexIn( urlStr, 0 ) != -1 )
				flvList << tempList;
			else
				flvListBefore20100323 << tempList;
			if ( tempList.count() >= SEARCH_AT_ONCE ) {
				QRegExp prefix( "^(.*&start=)(\\d+)$", Qt::CaseInsensitive );
				if ( prefix.indexIn( urlStr, 0 ) != -1 ) {
					QString cap1 = prefix.cap( 1 );
					int cap2 = prefix.cap( 2 ).toInt();
					QUrl url( cap1 + QString::number( cap2 + SEARCH_AT_ONCE ) );
					QNetworkRequest request( url );
					QNetworkReply* reply = manager.get( request );
					currentDownloads.append( reply );
				}
			}
		} else {
			QString page( reply->readAll() );
			//QString rx = reread ? "audio_player2.swf.type=real&m_name=([^\"]*)" : "video_player.swf.type=real&m_name=([^\"]*)";
			QString rx = reread ? "mp3player.swf.type=real&m_name=([^&\"]*)" : "video_player_wide.swf.type=real&m_name=([^\"]*)";
			QRegExp regexp( rx, Qt::CaseInsensitive );
			if ( regexp.indexIn( page ) > -1 && !flvList.contains( regexp.cap( 1 ) ) )
				flvList << regexp.cap( 1 );
		}
	}

	currentDownloads.removeAll(reply);
	reply->deleteLater();

	if (currentDownloads.isEmpty()) {
		manager.disconnect();
		eventLoop.exit();
	}
}
