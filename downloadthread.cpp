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

#include <stdlib.h>

#include "downloadthread.h"
#include "ui_mainwindow.h"
#include "downloadmanager.h"
#include "customizedialog.h"
#include "mainwindow.h"
#include "urldownloader.h"
#include "utility.h"
#include "mp3.h"
#include "qt4qt5.h"

#include <QCheckBox>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QXmlQuery>
#include <QTextCodec>
#include <QTemporaryFile>
#include <QDateTime>
#include <QEventLoop>
#include <QRegExp>
#include <QScriptEngine>
#include <QTextStream>
#include <QDate>
#include <QLocale>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QtNetwork>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QJsonValue>

#ifdef QT4_QT5_WIN
#define TimeOut " -m 10000 "
#else
#define TimeOut " -m 10 "
#endif

#define ScrambleLength 14
#define FlvMinSize 100	// ストリーミングが存在しなかった場合は13バイトだが少し大きめに設定
#define OriginalFormat "ts"
#define FilterOption "-bsf:a aac_adtstoasc"
#define CancelCheckTimeOut 500	// msec
#define DebugLog(s) if ( ui->toolButton_detailed_message->isChecked() ) {emit information((s));}

//--------------------------------------------------------------------------------
QString DownloadThread::prefix = "http://cgi2.nhk.or.jp/gogaku/st/xml/";
QString DownloadThread::suffix = "listdataflv.xml";
QString DownloadThread::json_prefix = "https://www.nhk.or.jp/radioondemand/json/";

QString DownloadThread::prefix1 = "https://nhk-vh.akamaihd.net/i/gogaku-stream/mp4/";
QString DownloadThread::prefix2 = "https://nhks-vh.akamaihd.net/i/gogaku-stream/mp4/";
QString DownloadThread::prefix3 = "https://vod-stream.nhk.jp/gogaku-stream/mp4/";
QString DownloadThread::suffix1 = "/master.m3u8";
QString DownloadThread::suffix2 = ".mp4/master.m3u8";
QString DownloadThread::suffix3 = "/index.m3u8";

QString DownloadThread::flv_host = "flv.nhk.or.jp";
QString DownloadThread::flv_app = "ondemand/";
QString DownloadThread::flv_service_prefix = "mp4:flv/gogaku/streaming/mp4/";

QString DownloadThread::flvstreamer;
QString DownloadThread::ffmpeg;
QString DownloadThread::test;
QString DownloadThread::scramble;
QString DownloadThread::optional1;
QString DownloadThread::optional2;
QString DownloadThread::optional3;
QString DownloadThread::optional4;
QString DownloadThread::opt_title1;
QString DownloadThread::opt_title2;
QString DownloadThread::opt_title3;
QString DownloadThread::opt_title4;
QStringList DownloadThread::malformed = (QStringList() << "3g2" << "3gp" << "m4a" << "mov");

QHash<QString, QString> DownloadThread::ffmpegHash;
QHash<QProcess::ProcessError, QString> DownloadThread::processError;

//--------------------------------------------------------------------------------

DownloadThread::DownloadThread( Ui::MainWindowClass* ui ) : isCanceled(false), failed1935(false) {
	this->ui = ui;
	if ( ffmpegHash.empty() ) {
		ffmpegHash["3g2"] = "%1,-vn,-bsf,aac_adtstoasc,-acodec,copy,%2";
		ffmpegHash["3gp"] = "%1,-vn,-bsf,aac_adtstoasc,-acodec,copy,%2";
		ffmpegHash["aac"] = "%1,-vn,-acodec,copy,%2";
		ffmpegHash["avi"] = "%1,-id3v2_version,3,-metadata,title=%3,-metadata,artist=NHK,-metadata,album=%4,-metadata,date=%5,-metadata,genre=Speech,-vn,-acodec,copy,%2";
		ffmpegHash["m4a"] = "%1,-id3v2_version,3,-metadata,title=%3,-metadata,artist=NHK,-metadata,album=%4,-metadata,date=%5,-metadata,genre=Speech,-vn,-bsf,aac_adtstoasc,-acodec,copy,%2";
		ffmpegHash["mka"] = "%1,-id3v2_version,3,-metadata,title=%3,-metadata,artist=NHK,-metadata,album=%4,-metadata,date=%5,-metadata,genre=Speech,-vn,-acodec,copy,%2";
		ffmpegHash["mkv"] = "%1,-id3v2_version,3,-metadata,title=%3,-metadata,artist=NHK,-metadata,album=%4,-metadata,date=%5,-metadata,genre=Speech,-vn,-acodec,copy,%2";
		ffmpegHash["mov"] = "%1,-id3v2_version,3,-metadata,title=%3,-metadata,artist=NHK,-metadata,album=%4,-metadata,date=%5,-metadata,genre=Speech,-vn,-bsf,aac_adtstoasc,-acodec,copy,%2";
		ffmpegHash["mp3"] = "%1,-id3v2_version,3,-metadata,title=%3,-metadata,artist=NHK,-metadata,album=%4,-metadata,date=%5,-metadata,genre=Speech,-vn,-acodec,libmp3lame,%2";
		ffmpegHash["ts"] = "%1,-vn,-acodec,copy,%2";
		ffmpegHash["op0"] = "%1,-id3v2_version,3,-metadata,title=%3,-metadata,artist=NHK,-metadata,album=%4,-metadata,date=%5,-metadata,genre=Speech,-vn,-acodec:a,libmp3lame,-ab,64k,-ac,1,%2";
		ffmpegHash["op1"] = "%1,-id3v2_version,3,-metadata,title=%3,-metadata,artist=NHK,-metadata,album=%4,-metadata,date=%5,-metadata,genre=Speech,-vn,-acodec:a,libmp3lame,-ab,48k,-ar,24000,-ac,1,%2";
		ffmpegHash["op2"] = "%1,-id3v2_version,3,-metadata,title=%3,-metadata,artist=NHK,-metadata,album=%4,-metadata,date=%5,-metadata,genre=Speech,-vn,-acodec:a,libmp3lame,-ab,40k,-ac,1,%2";
		ffmpegHash["op3"] = "%1,-id3v2_version,3,-metadata,title=%3,-metadata,artist=NHK,-metadata,album=%4,-metadata,date=%5,-metadata,genre=Speech,-vn,-acodec:a,libmp3lame,-ab,32k,-ac,1,%2";
		ffmpegHash["op4"] = "%1,-id3v2_version,3,-metadata,title=%3,-metadata,artist=NHK,-metadata,album=%4,-metadata,date=%5,-metadata,genre=Speech,-vn,-acodec:a,libmp3lame,-ab,24k,-ar,22050,-ac,1,%2";
		ffmpegHash["op5"] = "%1,-id3v2_version,3,-metadata,title=%3,-metadata,artist=NHK,-metadata,album=%4,-metadata,date=%5,-metadata,genre=Speech,-vn,-acodec:a,libmp3lame,-ab,16k,-ar,22050,-ac,1,%2";
	}
	if ( processError.empty() ) {
		processError[QProcess::FailedToStart] = "FailedToStart";
		processError[QProcess::Crashed] = "Crashed";
		processError[QProcess::Timedout] = "Timedout";
		processError[QProcess::ReadError] = "ReadError";
		processError[QProcess::WriteError] = "WriteError";
		processError[QProcess::UnknownError] = "UnknownError";
	}
}

QStringList DownloadThread::getAttribute( QString url, QString attribute ) {
	const QString xmlUrl = "doc('" + url + "')/musicdata/music/" + attribute + "/string()";
	QStringList attributeList;
	QXmlQuery query;
	query.setQuery( xmlUrl );
	if ( query.isValid() )
		query.evaluateTo( &attributeList );
	return attributeList;
}

QStringList DownloadThread::getJsonData( QString url, QString attribute ) {
	QStringList attributeList;
	attributeList.clear() ;
    	QEventLoop eventLoop;
	QNetworkAccessManager mgr;
 	QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
	const QString jsonUrl = json_prefix + url + "/bangumi_" + url + "_01.json";
	QUrl url_json( jsonUrl );
	QNetworkRequest req;
	req.setUrl(url_json);
	QNetworkReply *reply = mgr.get(req);
	eventLoop.exec(); // blocks stack until "finished()" has been called
	
	if (reply->error() == QNetworkReply::NoError) {
		QString strReply = (QString)reply->readAll();
		QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
		QJsonObject jsonObject = jsonResponse.object();
		QJsonObject jsonObj = jsonResponse.object();
    
		QJsonArray jsonArray = jsonObject[ "main" ].toArray();
		QJsonObject objx2 = jsonObject[ "main" ].toObject();
		QString program_name = objx2[ "program_name" ].toString();
		QJsonArray detail_list2 = objx2[ "detail_list" ].toArray();
		QJsonArray detail_list = objx2[ "detail_list" ].toArray();

		foreach (const QJsonValue & value, detail_list) {
			QJsonObject objxx = value.toObject();
			QJsonArray file_list = objxx[ "file_list" ].toArray();					
			foreach (const QJsonValue & value, file_list) {
				QJsonObject objxx2 = value.toObject();
				QString file_title = objxx2[ "file_title" ].toString();
				QString file_name = objxx2[ "file_name" ].toString();
				QString onair_date = objxx2[ "onair_date" ].toString();
				QString open_time = objxx2[ "open_time" ].toString();
				QString year = open_time.left( 4 );				
					if ( attribute == "program_name" ) attributeList += program_name;
					if ( attribute == "file_title" ) attributeList += file_title;
					if ( attribute == "file_name" ) attributeList += file_name;
					if ( attribute == "onair_date" ) attributeList += onair_date;
					if ( attribute == "open_time" ) attributeList += year;
        		}					
		}
	}
	return attributeList;
}

bool DownloadThread::checkExecutable( QString path ) {
	QFileInfo fileInfo( path );
	
	if ( !fileInfo.exists() ) {
		emit critical( path + QString::fromUtf8( "が見つかりません。" ) );
		return false;
	} else if ( !fileInfo.isExecutable() ) {
		emit critical( path + QString::fromUtf8( "は実行可能ではありません。" ) );
		return false;
	}
	
	return true;
}

bool DownloadThread::isFfmpegAvailable( QString& path ) {
	path = Utility::applicationBundlePath() + "ffmpeg";
#ifdef QT4_QT5_WIN
	path += ".exe";
#endif
	return checkExecutable( path );
}


//通常ファイルが存在する場合のチェックのために末尾にセパレータはついていないこと
bool DownloadThread::checkOutputDir( QString dirPath ) {
	bool result = false;
	QFileInfo dirInfo( dirPath );

	if ( dirInfo.exists() ) {
		if ( !dirInfo.isDir() )
			emit critical( QString::fromUtf8( "「" ) + dirPath + QString::fromUtf8( "」が存在しますが、フォルダではありません。" ) );
		else if ( !dirInfo.isWritable() )
			emit critical( QString::fromUtf8( "「" ) + dirPath + QString::fromUtf8( "」フォルダが書き込み可能ではありません。" ) );
		else
			result = true;
	} else {
		QDir dir;
		if ( !dir.mkpath( dirPath ) )
			emit critical( QString::fromUtf8( "「" ) + dirPath + QString::fromUtf8( "」フォルダの作成に失敗しました。" ) );
		else
			result = true;
	}

	return result;
}

//--------------------------------------------------------------------------------

#if 0
void DownloadThread::downloadENews( bool re_read ) {
	emit current( QString::fromUtf8( "ニュースで英会話のhtmlを分析中" ) );
	DownloadManager manager( re_read, ui->toolButton_enews_past->isChecked() );
	manager.singleShot();
	qSort( manager.flvList.begin(), manager.flvList.end(), qGreater<QString>() );

	QString flvstreamer;
	if ( !checkFlvstreamer( flvstreamer ) )
		return;

	int selection = ui->comboBox_enews->currentIndex();
	bool saveMovie = !re_read && ( selection == ENewsSaveBoth || selection == ENewsSaveMovie );
	bool saveAudio = re_read || selection == ENewsSaveBoth || selection == ENewsSaveAudio;

	QString kouza = QString::fromUtf8( re_read ? "ニュースで英会話（読み直し音声）" : "ニュースで英会話" );
	QString outputDir = MainWindow::outputDir + kouza;
	if ( !checkOutputDir( outputDir ) )
		return;
	outputDir += QDir::separator();	//通常ファイルが存在する場合のチェックのために後から追加する
	QString flv_host( "flv9.nhk.or.jp");
	QString flv_app( "flv9/_definst_/" );
	QString flv_service_prefix_20090330( re_read ? "mp3:e-news-flv/" : "e-news-flv/" );
	QString flv_service_prefix_20090728( re_read ? "mp3:e-news-flv/" : "e-news/data/" );
#ifdef QT4_QT5_WIN
	QString null( "nul" );
#else
	QString null( "/dev/null" );
#endif
	bool skip = ui->toolButton_skip->isChecked();

	QStringList flvListBefore20100323;
	QStringList flvListBefore20090728;
	foreach( QString flv, manager.flvListBefore20100323 ) {
		QDate theDay( flv.mid( 0, 4 ).toInt(), flv.mid( 4, 2 ).toInt(), flv.mid( 7, 2 ).toInt() );
		(theDay >= QDate( 2009, 7, 28 ) ? flvListBefore20100323 : flvListBefore20090728) << flv;
	}

	QList<QStringList> listList;
	listList << manager.flvList << flvListBefore20100323 << flvListBefore20090728;

	foreach( QStringList list, listList ) {
		if ( isCanceled )
			break;
		list.sort();
		QStringListIterator i( list );
		i.toBack();
		while ( i.hasPrevious() ) {
			QString flv = i.previous();
			if ( isCanceled )
				break;
			int year = flv.mid( 0, 4 ).toInt();
			int month = flv.mid( 4, 2 ).toInt();
			int day = flv.mid( 7, 2 ).toInt();
			QDate theDay( year, month, day );
			QString flv_service_prefix = theDay >= QDate( 2009, 7, 28 ) ? flv_service_prefix_20090728 : flv_service_prefix_20090330;
			QString hdate = theDay.toString( "yyyy_MM_dd" );
			int slashIndex = flv.lastIndexOf( '/' );
			QString flv_file = outputDir + ( slashIndex == -1 ? flv : flv.mid( slashIndex + 1 ) ) + ".flv";
			QString mp3_file = outputDir + kouza + "_" + hdate + ".mp3";
			QString movie_file = outputDir + kouza + "_" + hdate + ".flv";
			bool mp3Exists = QFile::exists( mp3_file );
			bool movieExists = QFile::exists( movie_file );

			if ( !skip || ( saveAudio && !mp3Exists ) || ( saveMovie && !movieExists ) ) {
				QString command1935 = QString( "\"%1\"%2 -r \"rtmp://%3/%4%5%6\" -o \"%7\" > %8" )
						.arg( flvstreamer, TimeOut, flv_host, flv_app, flv_service_prefix, flv, flv_file, null );
			QString command80 = QString( "\"%1\"%2 -r \"rtmpt://%3:80/%4%5%6\" -o \"%7\" > %8" )
						.arg( flvstreamer, TimeOut, flv_host, flv_app, flv_service_prefix, flv, flv_file, null );
			QString commandResume = QString( "\"%1\"%2 -r \"rtmpt://%3:80/%4%5%6\" -o \"%7\" --resume > %8" )
						.arg( flvstreamer, TimeOut, flv_host, flv_app, flv_service_prefix, flv, flv_file, null );
				QProcess process;
				emit current( QString::fromUtf8( "レコーディング中：　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
			int exitCode = 0;
			if ( !failed1935 && !isCanceled ) {
				if ( (exitCode = process.execute( command1935 )) != 0 )
					failed1935 = true;
			}
			if ( (failed1935 || exitCode) && !isCanceled )
				exitCode = process.execute( command80 );

				bool keep_on_error = ui->toolButton_keep_on_error->isChecked();
				if ( exitCode && !isCanceled ) {
					QFile flv( flv_file );
					if ( flv.size() > 0 ) {
						for ( int count = 0; exitCode && count < 5 && !isCanceled; count++ ) {
							emit current( QString::fromUtf8( "リトライ中：　　　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
							exitCode = process.execute( commandResume );
						}
					}
					if ( exitCode && !isCanceled ) {
						if ( !keep_on_error )
							flv.remove();
						emit critical( QString::fromUtf8( "レコーディングを完了できませんでした：　" ) +
									   kouza + QString::fromUtf8( "　" ) + hdate );
					}
				}

				if ( ( !exitCode || keep_on_error ) && !isCanceled ) {
					if ( saveAudio && ( !skip || !mp3Exists ) ) {
						QString error;
						if ( MP3::flv2mp3( flv_file, mp3_file, error ) ) {
							if ( !MP3::id3tag( mp3_file, kouza, kouza + "_" + hdate, flv.left( 4 ), "NHK", error ) )
								emit critical( error );
						} else
							emit critical( error );
					}
					if ( saveMovie && ( !skip || !movieExists ) ) {
						if ( movieExists ) {
							if ( !QFile::remove( movie_file ) ) {
								emit critical( QString::fromUtf8( "古い動画ファイルの削除に失敗しました：　" ) +
											   kouza + QString::fromUtf8( "　" ) + movie_file );
							} else
								movieExists = false;
						}
						if ( !movieExists && !QFile::rename( flv_file, movie_file ) ) {
							emit critical( QString::fromUtf8( "動画ファイル名の変更に失敗しました：　" ) +
										   kouza + QString::fromUtf8( "　" ) + flv_file );
						}
					}
				}
				if ( QFile::exists( flv_file ) )
					QFile::remove( flv_file );
			} else
				emit current( QString::fromUtf8( "スキップ：　　　　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
		}
	}
}
#endif

//--------------------------------------------------------------------------------

QStringList DownloadThread::getElements( QString url, QString path ) {
	const QString xmlUrl = "doc('" + url + "')" + path;
	QStringList elementList;
	QXmlQuery query;
	query.setQuery( xmlUrl );
	if ( query.isValid() )
		query.evaluateTo( &elementList );
	return elementList;
}

#if 0
void DownloadThread::downloadShower() {
	QString flvstreamer;
	if ( !checkFlvstreamer( flvstreamer ) )
		return;
		
	int selection = ui->comboBox_shower->currentIndex();
	bool saveMovie = selection == ENewsSaveBoth || selection == ENewsSaveMovie;
	bool saveAudio = selection == ENewsSaveBoth || selection == ENewsSaveAudio;
	
	QString kouza = QString::fromUtf8( "ABCニュースシャワー" );
	QString outputDir = MainWindow::outputDir + kouza;
	if ( !checkOutputDir( outputDir ) )
		return;
	outputDir += QDir::separator();	//通常ファイルが存在する場合のチェックのために後から追加する
	QString flv_host = "flv.nhk.or.jp";
	QString flv_app = "ondemand/flv/";
	QString flv_service_prefix( "worldwave/common/movie/" );
#ifdef QT4_QT5_WIN
	QString null( "nul" );
#else
	QString null( "/dev/null" );
#endif
	bool skip = ui->toolButton_skip->isChecked();

	// 当月と前月のxmlから可能なものをダウンロードする
	const char* prototype = "http://www.nhk.or.jp/worldwave/xml/abc_news_%1.xml";
	QString this_month = QString( prototype ).arg( QDate::currentDate().toString( "yyyyMM" ) );
	QString last_month = QString( prototype ).arg( QDate::currentDate().addMonths( -1 ).toString( "yyyyMM" ) );
	QStringList elements = getElements( this_month, "/rss/channel/item/movie/string()" );
	elements += getElements( last_month, "/rss/channel/item/movie/string()" );

	foreach (const QString &element, elements) {
		if ( isCanceled )
			break;
		static QStringList months = QStringList()
				<< "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun"
				<< "Jul" << "Aug" << "Sep" << "Oct" << "Nov" << "Dec";
		int year = 2000 + element.left( 2 ).toInt();
		int month = element.mid( 2, 2 ).toInt();
		int day = element.right( 2 ).toInt();
		QDate date( year, month, day );
		QString hdate = date.toString( "yyyy_MM_dd" );
		QString flv_file = outputDir + kouza + "_" + hdate + ".flv";
		QString mp3_file = outputDir + kouza + "_" + hdate + ".mp3";
		QString server_file = "abc" + date.toString( "yyMMdd" ) + ".flv";
		bool flvExists = QFile::exists( flv_file );
		bool mp3Exists = QFile::exists( mp3_file );

		if ( !skip || ( saveAudio && !mp3Exists ) || ( saveMovie && !flvExists ) ) {
			QString command1935 = QString( "\"%1\"%2 -r \"rtmp://%3/%4%5%6\" -o \"%7\" > %8" )
					.arg( flvstreamer, TimeOut, flv_host, flv_app, flv_service_prefix, server_file, flv_file, null );
			QString command80 = QString( "\"%1\"%2 -r \"rtmpt://%3:80/%4%5%6\" -o \"%7\" > %8" )
					.arg( flvstreamer, TimeOut, flv_host, flv_app, flv_service_prefix, server_file, flv_file, null );
			QString commandResume = QString( "\"%1\"%2 -r \"rtmpt://%3:80/%4%5%6\" -o \"%7\" --resume > %8" )
					.arg( flvstreamer, TimeOut, flv_host, flv_app, flv_service_prefix, server_file, flv_file, null );
			QProcess process;
			emit current( QString::fromUtf8( "レコーディング中：　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
			int exitCode = 0;
			if ( !failed1935 && !isCanceled ) {
				if ( (exitCode = process.execute( command1935 )) != 0 )
					failed1935 = true;
			}
			if ( (failed1935 || exitCode) && !isCanceled )
				exitCode = process.execute( command80 );

			bool keep_on_error = ui->toolButton_keep_on_error->isChecked();
			if ( exitCode && !isCanceled ) {
				QFile flv( flv_file );
				if ( flv.size() > 0 ) {
					for ( int count = 0; exitCode && count < 5 && !isCanceled; count++ ) {
						emit current( QString::fromUtf8( "リトライ中：　　　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
						exitCode = process.execute( commandResume );
					}
				}
				if ( exitCode && !isCanceled ) {
					if ( !keep_on_error )
						flv.remove();
					emit critical( QString::fromUtf8( "レコーディングを完了できませんでした：　" ) +
							kouza + QString::fromUtf8( "　" ) + hdate );
				}
			}

			if ( ( !exitCode || keep_on_error ) && !isCanceled ) {
				if ( saveAudio && ( !skip || !mp3Exists ) ) {
					QString error;
					if ( MP3::flv2mp3( flv_file, mp3_file, error ) ) {
						if ( !MP3::id3tag( mp3_file, kouza, kouza + "_" + hdate, date.toString( "yyyy" ), "NHK", error ) )
							emit critical( error );
					} else
						emit critical( error );
				}
			}
			if ( !saveMovie && QFile::exists( flv_file ) )
				QFile::remove( flv_file );
		} else
			emit current( QString::fromUtf8( "スキップ：　　　　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
	}
}
#endif

//--------------------------------------------------------------------------------

QStringList one2two( QStringList hdateList ) {
	QStringList result;
	QRegExp rx("(\\d+)(?:\\D+)(\\d+)");

	for ( int i = 0; i < hdateList.count(); i++ ) {
		QString hdate = hdateList[i];
		if ( rx.indexIn( hdate, 0 ) != -1 ) {
			uint length = rx.cap( 2 ).length();
			if ( length == 1 )
				hdate.replace( rx.pos( 2 ), 1, "0" + rx.cap( 2 ) );
			length = rx.cap( 1 ).length();
			if ( length == 1 )
				hdate.replace( rx.pos( 1 ), 1, "0" + rx.cap( 1 ) );
		}
		result << hdate;
	}

	return result;
}

QStringList one2two2( QStringList hdateList2 ) {
	QStringList result;
	QRegExp rx("(\\d+)(?:\\D+)(\\d+)");

	for ( int i = 0; i < hdateList2.count(); i++ ) {
		QString hdate = hdateList2[i];
		if ( rx.indexIn( hdate, 0 ) != -1 ) {
			uint length = rx.cap( 2 ).length();
			if ( length == 1 )
				hdate.replace( rx.pos( 2 ), 1, "0" + rx.cap( 2 ) );
			length = rx.cap( 1 ).length();
			if ( length == 1 )
				hdate.replace( rx.pos( 1 ), 1, "0" + rx.cap( 1 ) );
		}
		QString month2 = hdate.left( 2 );
		QString day2 = hdate.mid( 3, 2 );
		QDate today;
		today.setDate(QDate::currentDate().year(), QDate::currentDate().month(), QDate::currentDate().day());
		QDateTime dt = QDateTime::fromString( month2 + "/" + day2, "MM/dd" ).addDays(7);
	
		QString str1 = dt.toString("MM");
		QString str2 = dt.toString("dd");


			hdate.replace( day2, str2 );
			hdate.replace( month2, str1 );

		result << hdate;
	}

	return result;
}

QStringList thisweekfile( QStringList fileList2, QStringList codeList ) {
	QStringList result;
	
	for ( int i = 0; i < fileList2.count(); i++ ) {
		QString filex = fileList2[i];
		int filexxx = codeList[i].toInt() + fileList2.count() ;
		filex.replace( codeList[i].right( 3 ) ,  QString::number( filexxx ).right( 3 ) );
		filex.remove( "-re01" );
			
		result << filex;
	}

	return result;
}


//--------------------------------------------------------------------------------

bool illegal( char c ) {
	bool result = false;
	switch ( c ) {
	case '/':
	case '\\':
	case ':':
	case '*':
	case '?':
	case '"':
	case '<':
	case '>':
	case '|':
	case '#':
	case '{':
	case '}':
	case '%':
	case '&':
	case '~':
		result = true;
		break;
	default:
		break;
	}
	return result;
}

QString DownloadThread::formatName( QString format, QString kouza, QString hdate, QString file, QString nendo, bool checkIllegal ) {
	int month = hdate.left( 2 ).toInt();
	int year = nendo.right( 4 ).toInt();
	int day = hdate.mid( 3, 2 ).toInt();
	int year1 = QDate::currentDate().year();

	if ( QString::compare(  kouza , QString::fromUtf8( "ボキャブライダー" ) ) ==0 ){
		if ( month == 3 && ( day == 29 || day == 30 || day == 31) && year == 2022 ) 
		year += 0;
 		else
		if ( month < 4 )
		year += 1;
	} else {
	if ( month <= 4 && QDate::currentDate().year() > year )
		year = year + (year1 - year);
	}

	if ( file.right( 4 ) == ".flv" )
		file = file.left( file.length() - 4 );

	QString result;

	bool percent = false;
	for ( int i = 0; i < format.length(); i++ ) {
		QChar qchar = format[i];
		if ( percent ) {
			percent = false;
			char ascii = qchar.toLatin1();
			if ( checkIllegal && illegal( ascii ) )
				continue;
			switch ( ascii ) {
			case 'k': result += kouza; break;
			case 'h': result += hdate; break;
			case 'f': result += file; break;
			//case 'r': result += MainWindow::applicationDirPath(); break;
			//case 'p': result += QDir::separator(); break;
			case 'Y': result += QString::number( year ); break;
			case 'y': result += QString::number( year ).right( 2 ); break;
			case 'N': result += nendo + QString::fromUtf8( "年度" ); break;
			case 'n': result += nendo.right( 2 ) + QString::fromUtf8( "年度" ); break;
			case 'M': result += QString::number( month + 100 ).right( 2 ); break;
			case 'm': result += QString::number( month ); break;
			case 'D': result += QString::number( day + 100 ).right( 2 ); break;
			case 'd': result += QString::number( day ); break;
			default: result += qchar; break;
			}
		} else {
			if ( qchar == QChar( '%' ) )
				percent = true;
			else if ( checkIllegal && illegal( qchar.toLatin1() ) )
				continue;
			else
				result += qchar;
		}
	}

	return result;
}

//--------------------------------------------------------------------------------

bool DownloadThread::captureStream( QString kouza, QString hdate, QString file, QString nendo, QString this_week ) {
	QString outputDir = MainWindow::outputDir + kouza;
	if ( QString::compare( this_week, "今週放送分" ) ==0 ){
		outputDir = outputDir + "/" + QString::fromUtf8( "今週放送分" );
		QDate today;
		today.setDate(QDate::currentDate().year(),QDate::currentDate().month(),QDate::currentDate().day());
		if ( QString::compare(  kouza , QString::fromUtf8( "ボキャブライダー" ) ) ==0 || (today.dayOfWeek() >= 1 && today.dayOfWeek() <= 2))
		return true;
	}
		
	if ( !checkOutputDir( outputDir ) )
		return false;
	outputDir += QDir::separator();	//通常ファイルが存在する場合のチェックのために後から追加する

	QString titleFormat;
	QString fileNameFormat;
	CustomizeDialog::formats( kouza, titleFormat, fileNameFormat );
	QString id3tagTitle = formatName( titleFormat, kouza, hdate, file, nendo, false );
	QString outFileName = formatName( fileNameFormat, kouza, hdate, file, nendo, true );
	QFileInfo fileInfo( outFileName );
	QString outBasename = fileInfo.completeBaseName();
	
	// 2013/04/05 オーディオフォーマットの変更に伴って拡張子の指定に対応
	QString extension = ui->comboBox_extension->currentText();
	QString extension1 = extension;
	if ( extension.left( 2 ) == "op" ) extension1 = "mp3";
	outFileName = outBasename + "." + extension1;

#ifdef QT4_QT5_WIN
	QString null( "nul" );
#else
	QString null( "/dev/null" );
#endif
	if ( QString::compare(  kouza , QString::fromUtf8( "ボキャブライダー" ) ) ==0 ){

	}

	int month = hdate.left( 2 ).toInt();
	int year = nendo.right( 4 ).toInt();
	int day = hdate.mid( 3, 2 ).toInt();
	if ( 2022 > year ) return false;
	int year1 = QDate::currentDate().year();

	if ( QString::compare(  kouza , QString::fromUtf8( "ボキャブライダー" ) ) ==0 ){
		if ( month == 3 && ( day == 29 || day == 30 || day == 31) && year == 2022 ) 
		year += 0; 
 		else
		if ( month < 4 )
		year += 1;
	} else {
	if ( month <= 4 && QDate::currentDate().year() > year )
		year = year + (year1 - year);
	}
	QDate onair( year, month, day );
	QString yyyymmdd = onair.toString( "yyyy_MM_dd" );

	QString kon_nendo = "2022"; //QString::number(year1);

	if ( QString::compare(  kouza , QString::fromUtf8( "ボキャブライダー" ) ) ==0 ){
		QDate today;
		today.setDate(QDate::currentDate().year(),QDate::currentDate().month(),QDate::currentDate().day());
		int day2 = onair.daysTo(QDate::currentDate())-today.dayOfWeek();
//		if ( ui->toolButton_vrradio->isChecked() && !ui->toolButton_vrradio1->isChecked() ) {
//			if ( day2 > 7 || day2 < 0 ) return false;
//		}
//		if ( !ui->toolButton_vrradio->isChecked() && ui->toolButton_vrradio1->isChecked() ) {
		if ( ui->toolButton_vrradio1->isChecked() ) {
			if ( day2 > 0 || day2 < -7 ) return false;
		}
//		if ( ui->toolButton_vrradio->isChecked() && ui->toolButton_vrradio1->isChecked() ) {
//			if ( !QString::compare( kon_nendo , nendo ) == 0 ) return false;
//		}
	}
	
	if ( ui->toolButton_skip->isChecked() && QFile::exists( outputDir + outFileName ) ) {
		emit current( QString::fromUtf8( "スキップ：　　　　　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
		return true;
	}
  	emit current( QString::fromUtf8( "レコーディング中：　　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
	
	Q_ASSERT( ffmpegHash.contains( extension ) );
	QString dstPath;
#ifdef QT4_QT5_WIN
	if ( true ) {
		QTemporaryFile file;
		if ( file.open() ) {
			dstPath = file.fileName() + "." + extension1;
			file.close();
		} else {
			emit critical( QString::fromUtf8( "一時ファイルの作成に失敗しました：　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
			return false;
		}
	}
#else
	dstPath = outputDir + outFileName;
#endif
	QString filem3u8a; QString filem3u8b;
	if ( file.right(4) != ".mp4" ) {
		filem3u8a = prefix1 + file + ".mp4/master.m3u8";
		filem3u8b = prefix2 + file + ".mp4/master.m3u8";
	} else {
		filem3u8a = prefix1 + file + "/master.m3u8";
		filem3u8b = prefix2 + file + "/master.m3u8";
	}
	QString filem3u8c = prefix3 + file  + "/index.m3u8";
	QStringList arguments_v = { "-http_seekable", "0", "-version", "0" };
	QProcess process_v;
	process_v.setProgram( ffmpeg );
	process_v.setArguments( arguments_v );
	process_v.start();
	process_v.waitForFinished();
	QString str_v = process_v.readAllStandardError();
	process_v.kill();
	process_v.close();	 
	QString arguments00 = "-y -http_seekable 0 -i";
	if (str_v.contains( "Option not found" )) {
	                     arguments00 = "-y -i";
	}

	QStringList arguments0 = arguments00.split(" ");
	QStringList arguments = arguments0 + ffmpegHash[extension]
			.arg( filem3u8a, dstPath, id3tagTitle, kouza, QString::number( year ) ).split(",");
	QStringList arguments2 = arguments0 + ffmpegHash[extension]
			.arg( filem3u8b, dstPath, id3tagTitle, kouza, QString::number( year ) ).split(","); 
	QStringList arguments3 = arguments0 + ffmpegHash[extension]
			.arg( filem3u8c, dstPath, id3tagTitle, kouza, QString::number( year ) ).split(","); 

	//qDebug() << commandFfmpeg;
	//DebugLog( commandFfmpeg );
	QProcess process;
	process.setProgram( ffmpeg );
	process.setArguments( arguments );
	
	QProcess process2;
	process2.setProgram( ffmpeg );
	process2.setArguments( arguments2 );
	
	QProcess process3;
	process3.setProgram( ffmpeg );
	process3.setArguments( arguments3 );
	
	process.start();
	if ( !process.waitForStarted( -1 ) ) {
		emit critical( QString::fromUtf8( "ffmpeg起動エラー(%3)：　%1　　%2" )
				.arg( kouza, yyyymmdd,  processError[process.error()] ) );
		QFile::remove( dstPath );
		return false;
	}

	// ユーザのキャンセルを確認しながらffmpegの終了を待つ
	while ( !process.waitForFinished( CancelCheckTimeOut ) ) {
		// キャンセルボタンが押されていたらffmpegをkillし、ファイルを削除してリターン
		if ( isCanceled ) {
			process.kill();
			QFile::remove( dstPath );
			return false;
		}
		// 単なるタイムアウトは継続
		if ( process.error() == QProcess::Timedout )
			continue;
		// エラー発生時はメッセージを表示し、出力ファイルを削除してリターン
		emit critical( QString::fromUtf8( "ffmpeg実行エラー(%3)：　%1　　%2" )
				.arg( kouza, yyyymmdd,  processError[process.error()] ) );
		QFile::remove( dstPath );
		return false;
	}


	QString ffmpeg_Error;
	ffmpeg_Error.append(process.readAllStandardError());

	// ffmpeg終了ステータスに応じた処理をしてリターン
	if ( process.exitCode() || ffmpeg_Error.contains("HTTP error") || ffmpeg_Error.contains("Unable to open resource:") ) {
	process.kill();
	process.close();
	process2.start();

		if ( !process2.waitForStarted( -1 ) ) {
			emit critical( QString::fromUtf8( "ffmpeg起動エラー2(%3)：　%1　　%2" )
					.arg( kouza, yyyymmdd,  processError[process2.error()] ) );
			QFile::remove( dstPath );
			return false;
		}

	// ユーザのキャンセルを確認しながらffmpegの終了を待つ
		while ( !process2.waitForFinished( CancelCheckTimeOut ) ) {
		// キャンセルボタンが押されていたらffmpegをkillし、ファイルを削除してリターン
			if ( isCanceled ) {
				process2.kill();
				QFile::remove( dstPath );
				return false;
			}
		// 単なるタイムアウトは継続
			if ( process2.error() == QProcess::Timedout )
				continue;
		// エラー発生時はメッセージを表示し、出力ファイルを削除してリターン
			emit critical( QString::fromUtf8( "ffmpeg実行エラー2(%3)：　%1　　%2" )
					.arg( kouza, yyyymmdd,  processError[process2.error()] ) );
			QFile::remove( dstPath );
			return false;
		}

	QString ffmpeg_Error2;
	ffmpeg_Error2.append(process2.readAllStandardError());

	// ffmpeg終了ステータスに応じた処理をしてリターン
	if ( process2.exitCode() || ffmpeg_Error2.contains("HTTP error") || ffmpeg_Error2.contains("Unable to open resource:") ) {
	process2.kill();
	process2.close();
	process3.start();

		if ( !process3.waitForStarted( -1 ) ) {
			emit critical( QString::fromUtf8( "ffmpeg起動エラー(%3)：　%1　　%2" )
					.arg( kouza, yyyymmdd,  processError[process3.error()] ) );
			QFile::remove( dstPath );
			return false;
		}

	// ユーザのキャンセルを確認しながらffmpegの終了を待つ
		while ( !process3.waitForFinished( CancelCheckTimeOut ) ) {
		// キャンセルボタンが押されていたらffmpegをkillし、ファイルを削除してリターン
			if ( isCanceled ) {
				process3.kill();
				QFile::remove( dstPath );
				return false;
			}
		// 単なるタイムアウトは継続
			if ( process3.error() == QProcess::Timedout )
				continue;
		// エラー発生時はメッセージを表示し、出力ファイルを削除してリターン
			emit critical( QString::fromUtf8( "ffmpeg実行エラー(%3)：　%1　　%2" )
					.arg( kouza, yyyymmdd,  processError[process3.error()] ) );
			QFile::remove( dstPath );
			return false;
		}
	
	QString ffmpeg_Error3;
	ffmpeg_Error3.append(process3.readAllStandardError());
	
	// ffmpeg終了ステータスに応じた処理をしてリターン
	if ( process3.exitCode() || ffmpeg_Error3.contains("HTTP error") || ffmpeg_Error3.contains("Unable to open resource:") ) {	
				emit critical( QString::fromUtf8( "レコーディング失敗：　%1　　%2" ).arg( kouza, yyyymmdd ) );
			QFile::remove( dstPath );
			return false;
		}
	}}
#ifdef QT4_QT5_WIN
		QFile::rename( dstPath, outputDir + outFileName );
#endif
			return true;
}

bool DownloadThread::captureStream_json( QString kouza, QString hdate, QString file, QString nendo, QString title, QString this_week ) {
	QString outputDir = MainWindow::outputDir + kouza;
	if ( this_week == "R" )
		outputDir = MainWindow::outputDir + QString::fromUtf8( "[聴逃]" )+ "/" + kouza;

	if ( !checkOutputDir( outputDir ) )
		return false;
	outputDir += QDir::separator();	//通常ファイルが存在する場合のチェックのために後から追加する

	QString titleFormat;
	QString fileNameFormat;
	CustomizeDialog::formats( kouza, titleFormat, fileNameFormat );
	QString id3tagTitle = title;
	QString outFileName = formatName( fileNameFormat, kouza, hdate, file, nendo, true );
	QFileInfo fileInfo( outFileName );
	QString outBasename = fileInfo.completeBaseName();
	
	// 2013/04/05 オーディオフォーマットの変更に伴って拡張子の指定に対応
	QString extension = ui->comboBox_extension->currentText();
	QString extension1 = extension;
	if ( extension.left( 2 ) == "op" ) extension1 = "mp3";
	outFileName = outBasename + "." + extension1;

#ifdef QT4_QT5_WIN
	QString null( "nul" );
#else
	QString null( "/dev/null" );
#endif
	int month = hdate.left( 2 ).toInt();
	int year = nendo.right( 4 ).toInt();
	int day = hdate.mid( 3, 2 ).toInt();
	if ( 2022 > year ) return false;
	int year1 = QDate::currentDate().year();

	if ( month <= 4 && QDate::currentDate().year() > year )
		year = year + (year1 - year);

	QDate onair( year, month, day );
	QString yyyymmdd = onair.toString( "yyyy_MM_dd" );

	QString kon_nendo = "2022"; //QString::number(year1);
	
	if ( ui->toolButton_skip->isChecked() && QFile::exists( outputDir + outFileName ) ) {
	   if ( this_week == "R" ) {
		emit current( QString::fromUtf8( "スキップ：[聴逃]　　　　　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
	   } else {
		emit current( QString::fromUtf8( "スキップ：　　　　　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
	   }
		return true;
	}
	
	if ( this_week == "R" ) {
	  	emit current( QString::fromUtf8( "レコーディング中：[聴逃]　　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
	} else {
  		emit current( QString::fromUtf8( "レコーディング中：　　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
	}
	
	Q_ASSERT( ffmpegHash.contains( extension ) );
	QString dstPath;
#ifdef QT4_QT5_WIN
	if ( true ) {
		QTemporaryFile file;
		if ( file.open() ) {
			dstPath = file.fileName() + "." + extension1;
			file.close();
		} else {
			emit critical( QString::fromUtf8( "一時ファイルの作成に失敗しました：　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
			return false;
		}
	}
#else
	dstPath = outputDir + outFileName;
#endif

	QStringList arguments_v = { "-http_seekable", "0", "-version", "0" };
	QProcess process_v;
	process_v.setProgram( ffmpeg );
	process_v.setArguments( arguments_v );
	process_v.start();
	process_v.waitForFinished();
	QString str_v = process_v.readAllStandardError();
	process_v.kill();
	process_v.close();	 
	QString arguments00 = "-y -http_seekable 0 -i";
	if (str_v.contains( "Option not found" )) {
	                     arguments00 = "-y -i";
	}
				
	QStringList arguments0 = arguments00.split(" ");
	QString filem3u8aA = file;
	QString id3tagTitleA = id3tagTitle;
	QString kouzaA = kouza;	
	
	QStringList argumentsA = arguments0 + ffmpegHash[extension]
			.arg( filem3u8aA, dstPath, id3tagTitleA, kouzaA,  nendo ).split(",");
	QProcess process;
	process.setProgram( ffmpeg );
	process.setArguments( argumentsA );
	process.start();

	if ( !process.waitForStarted( -1 ) ) {
		emit critical( QString::fromUtf8( "ffmpeg起動エラー(%3)：　%1　　%2" )
				.arg( kouza, yyyymmdd,  processError[process.error()] ) );
		QFile::remove( dstPath );
		return false;
	}

	// ユーザのキャンセルを確認しながらffmpegの終了を待つ
		while ( !process.waitForFinished( CancelCheckTimeOut ) ) {
		// キャンセルボタンが押されていたらffmpegをkillし、ファイルを削除してリターン
			if ( isCanceled ) {
				process.kill();
				QFile::remove( dstPath );
				return false;
			}
		// 単なるタイムアウトは継続
			if ( process.error() == QProcess::Timedout )
				continue;
		// エラー発生時はメッセージを表示し、出力ファイルを削除してリターン
			emit critical( QString::fromUtf8( "ffmpeg実行エラー(%3)：　%1　　%2" )
					.arg( kouza, yyyymmdd,  processError[process.error()] ) );
			QFile::remove( dstPath );
			return false;
		}

	// ffmpeg終了ステータスに応じた処理をしてリターン
		if ( process.exitCode() ) {
			emit critical( QString::fromUtf8( "レコーディング失敗：　%1　　%2" ).arg( kouza, yyyymmdd ) );
			QFile::remove( dstPath );
			return false;
		}
		
		QString ffmpeg_Error;
		 ffmpeg_Error.append(process.readAllStandardError());
				
	if ( ffmpeg_Error.contains("HTTP error") ) 
			emit critical( QString::fromUtf8( "HTTP error" ));
	if ( ffmpeg_Error.contains("Unable to open resource:") ) 
			emit critical( QString::fromUtf8( "Unable to open resource:" ));
				
	if ( ffmpeg_Error.contains("HTTP error") || ffmpeg_Error.contains("Unable to open resource:") ) {
			emit critical( QString::fromUtf8( "レコーディング失敗：　%1　　%2" ).arg( kouza, yyyymmdd ) );
			QFile::remove( dstPath );
			return false;
		}

	// ffmpeg終了ステータスに応じた処理をしてリターン
	if ( process.exitCode() ) {
	process.kill();
	process.close();
	}
#ifdef QT4_QT5_WIN
		QFile::rename( dstPath, outputDir + outFileName );
#endif
			return true;
}




QString DownloadThread::paths[] = {
	"english/basic0", "english/basic1", "english/basic2", "english/basic3",
	"english/timetrial", "english/kaiwa", "english/business1",
	"english/enjoy", 
	"chinese/kouza", "chinese/stepup", "french/kouza", "french/kouza2",
	"italian/kouza", "italian/kouza2", "hangeul/kouza", "hangeul/stepup",
	"german/kouza", "german/kouza2", "spanish/kouza", "spanish/kouza2", "russian/kouza", "russian/kouza2", 
	"english/vr-radio",
	"null_optional1", "null_optional2", "null_optional3", "null_optional4"
};

QString DownloadThread::json_paths[] = {
	"0000", "6806", "6807", "6808",
	"2331", "0916", "6809",
	"3064",
	"0915", "6581", "0953", "4412",
	"0946", "4411", "0951", "6810",
	"0943", "4410", "0948", "4413", "0956", "4414", 
	"4121",
	"7512", "0937", "7629", "2769"
};

void DownloadThread::run() {
	QAbstractButton* checkbox[] = {
		ui->toolButton_basic0, ui->toolButton_basic1, ui->toolButton_basic2, ui->toolButton_basic3,
		ui->toolButton_timetrial, ui->toolButton_kaiwa, ui->toolButton_business1,
		ui->toolButton_enjoy,
		ui->toolButton_chinese, ui->toolButton_stepup_chinese, ui->toolButton_french, ui->toolButton_french,
		ui->toolButton_italian, ui->toolButton_italian, ui->toolButton_hangeul, ui->toolButton_stepup_hangeul,
		ui->toolButton_german, ui->toolButton_german, ui->toolButton_spanish,  ui->toolButton_spanish, 
		ui->toolButton_russian, ui->toolButton_russian, 
		ui->toolButton_vrradio1,
		ui->toolButton_optional1, ui->toolButton_optional2, 
		ui->toolButton_optional3, ui->toolButton_optional4, 
		NULL
	};

	if ( !isFfmpegAvailable( ffmpeg ) )
		return;

	//emit information( QString::fromUtf8( "2013年7月29日対応版です。" ) );
	//emit information( QString::fromUtf8( "ニュースで英会話とABCニュースシャワーは未対応です。" ) );
	//emit information( QString::fromUtf8( "----------------------------------------" ) );

	for ( int i = 0; checkbox[i] && !isCanceled; i++ ) {
	
//		optional1 = MainWindow::optional1;
//		optional2 = MainWindow::optional2;
//		optional3 = MainWindow::optional3;
//		optional4 = MainWindow::optional4;
//		if ( paths[i].right( 9 ).startsWith("optional1") ) json_paths[i] = optional1;
//		if ( paths[i].right( 9 ).startsWith("optional2") ) json_paths[i] = optional2;
//		if ( paths[i].right( 9 ).startsWith("optional3") ) json_paths[i] = optional3;
//		if ( paths[i].right( 9 ).startsWith("optional4") ) json_paths[i] = optional4;

		if ( checkbox[i]->isChecked() ) {
		   if ( (paths[i].left( 4 ) != "null" && !(ui->checkBox_next_week2->isChecked())) || json_paths[i] == "0000" ) {
			QStringList fileList = getAttribute( prefix + paths[i] + "/" + suffix, "@file" );
			QStringList kouzaList = getAttribute( prefix + paths[i] + "/" + suffix, "@kouza" );
			QStringList hdateList = one2two( getAttribute( prefix + paths[i] + "/" + suffix, "@hdate" ) );
			QStringList nendoList = getAttribute( prefix + paths[i] + "/" + suffix, "@nendo" );
			
			if ( fileList.count() && fileList.count() == kouzaList.count() && fileList.count() == hdateList.count() ) {
				if ( true /*ui->checkBox_this_week->isChecked()*/ ) {
					for ( int j = 0; j < fileList.count() && !isCanceled; j++ ){
						captureStream( kouzaList[j], hdateList[j], fileList[j], nendoList[j], "今週公開分" );
					}
				}
			}
		   }
		   if (paths[i].left( 4 ) != "null" &&  ui->checkBox_next_week2->isChecked() ) {
		   		QStringList fileList2 = getJsonData( json_paths[i], "file_name" );
				QStringList kouzaList2 = getJsonData( json_paths[i], "program_name" );
				QStringList file_titleList = getJsonData( json_paths[i], "file_title" );
				QStringList hdateList2 = one2two( getJsonData( json_paths[i], "onair_date" ));
				QStringList yearList = getJsonData( json_paths[i], "open_time" );
				QStringList kouzaList = getAttribute( prefix + paths[i] + "/" + suffix, "@kouza" );

				if ( fileList2.count() && fileList2.count() == kouzaList2.count() && fileList2.count() == hdateList2.count() ) {
					for ( int j = 0; j < fileList2.count() && !isCanceled; j++ ){
						captureStream_json( kouzaList2[j], hdateList2[j], fileList2[j], yearList[j], file_titleList[j], "R" );
					}
				}
		   }
		   if ( paths[i].left( 4 ) == "null" ) {
		   	QStringList fileList2 = getJsonData( json_paths[i], "file_name" );
			QStringList kouzaList2 = getJsonData( json_paths[i], "program_name" );
			QStringList file_titleList = getJsonData( json_paths[i], "file_title" );
			QStringList hdateList2 = one2two( getJsonData( json_paths[i], "onair_date" ));
			QStringList yearList = getJsonData( json_paths[i], "open_time" );
			
			if ( fileList2.count() && fileList2.count() == kouzaList2.count() && fileList2.count() == hdateList2.count() ) {
					for ( int j = 0; j < fileList2.count() && !isCanceled; j++ ){
						captureStream_json( kouzaList2[j], hdateList2[j], fileList2[j], yearList[j], file_titleList[j], "" );
					}
			}
		   }		   
	  }}	
	  
	//if ( !isCanceled && ui->checkBox_shower->isChecked() )
		//downloadShower();

	//if ( !isCanceled && ui->checkBox_14->isChecked() )
		//downloadENews( false );

	//if ( !isCanceled && ui->checkBox_15->isChecked() )
		//downloadENews( true );

	emit current( "" );
	//キャンセル時にはdisconnectされているのでemitしても何も起こらない
	emit information( QString::fromUtf8( "レコーディング作業が終了しました。" ) );
}
