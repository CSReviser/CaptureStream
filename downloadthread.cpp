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

#include <QCheckBox>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QXmlQuery>
#include <QProcess>
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
//#include <QtCrypto>

#ifdef Q_WS_WIN
#define Timeout " -m 10000 "
#else
#define Timeout " -m 10 "
#endif

#define ScrambleLength 14
#define FLV_MIN_SIZE 100	// ストリーミングが存在しなかった場合は13バイトだが少し大きめに設定
#define OriginalFormat "ts"
#define FilterOption "-bsf:a aac_adtstoasc"

DownloadThread::DownloadThread( Ui::MainWindowClass* ui ) : isCanceled(false), failed1935(false) {
	this->ui = ui;
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
#ifdef Q_WS_WIN
	path += ".exe";
#endif
	return checkExecutable( path );
}

bool DownloadThread::isOpensslAvailable( QString& path ) {
#ifdef Q_WS_WIN
	path = Utility::applicationBundlePath() + "openssl.exe";
#else
	path = "/usr/bin/openssl";
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
	DownloadManager manager( re_read, ui->checkBox_enews_past->isChecked() );
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
#ifdef Q_WS_WIN
	QString null( "nul" );
#else
	QString null( "/dev/null" );
#endif
	bool skip = ui->checkBox_skip->isChecked();

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
						.arg( flvstreamer, Timeout, flv_host, flv_app, flv_service_prefix, flv, flv_file, null );
			QString command80 = QString( "\"%1\"%2 -r \"rtmpt://%3:80/%4%5%6\" -o \"%7\" > %8" )
						.arg( flvstreamer, Timeout, flv_host, flv_app, flv_service_prefix, flv, flv_file, null );
			QString commandResume = QString( "\"%1\"%2 -r \"rtmpt://%3:80/%4%5%6\" -o \"%7\" --resume > %8" )
						.arg( flvstreamer, Timeout, flv_host, flv_app, flv_service_prefix, flv, flv_file, null );
				QProcess process;
				emit current( QString::fromUtf8( "ダウンロード中：　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
			int exitCode = 0;
			if ( !failed1935 && !isCanceled ) {
				if ( (exitCode = process.execute( command1935 )) != 0 )
					failed1935 = true;
			}
			if ( (failed1935 || exitCode) && !isCanceled )
				exitCode = process.execute( command80 );

				bool keep_on_error = ui->checkBox_keep_on_error->isChecked();
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
						emit critical( QString::fromUtf8( "ダウンロードを完了できませんでした：　" ) +
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
#ifdef Q_WS_WIN
	QString null( "nul" );
#else
	QString null( "/dev/null" );
#endif
	bool skip = ui->checkBox_skip->isChecked();

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
					.arg( flvstreamer, Timeout, flv_host, flv_app, flv_service_prefix, server_file, flv_file, null );
			QString command80 = QString( "\"%1\"%2 -r \"rtmpt://%3:80/%4%5%6\" -o \"%7\" > %8" )
					.arg( flvstreamer, Timeout, flv_host, flv_app, flv_service_prefix, server_file, flv_file, null );
			QString commandResume = QString( "\"%1\"%2 -r \"rtmpt://%3:80/%4%5%6\" -o \"%7\" --resume > %8" )
					.arg( flvstreamer, Timeout, flv_host, flv_app, flv_service_prefix, server_file, flv_file, null );
			QProcess process;
			emit current( QString::fromUtf8( "ダウンロード中：　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
			int exitCode = 0;
			if ( !failed1935 && !isCanceled ) {
				if ( (exitCode = process.execute( command1935 )) != 0 )
					failed1935 = true;
			}
			if ( (failed1935 || exitCode) && !isCanceled )
				exitCode = process.execute( command80 );

			bool keep_on_error = ui->checkBox_keep_on_error->isChecked();
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
					emit critical( QString::fromUtf8( "ダウンロードを完了できませんでした：　" ) +
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

QString DownloadThread::formatName( QString format, QString kouza, QString hdate, QString file, bool checkIllegal ) {
	int month = hdate.left( 2 ).toInt();
	int year = 2000 + file.left( 2 ).toInt();
	if ( month <= 4 && QDate::currentDate().year() > year )
		year += 1;
	int day = hdate.mid( 3, 2 ).toInt();

	if ( file.right( 4 ) == ".flv" )
		file = file.left( file.length() - 4 );

	QString result;

	bool percent = false;
	for ( int i = 0; i < format.length(); i++ ) {
		QChar qchar = format[i];
		if ( percent ) {
			percent = false;
			char ascii = qchar.toAscii();
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
			case 'M': result += QString::number( month + 100 ).right( 2 ); break;
			case 'm': result += QString::number( month ); break;
			case 'D': result += QString::number( day + 100 ).right( 2 ); break;
			case 'd': result += QString::number( day ); break;
			default: result += qchar; break;
			}
		} else {
			if ( qchar == QChar( '%' ) )
				percent = true;
			else if ( checkIllegal && illegal( qchar.toAscii() ) )
				continue;
			else
				result += qchar;
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
QString DownloadThread::prefix = "http://cgi2.nhk.or.jp/gogaku/";
QString DownloadThread::suffix = "listdataflv.xml";

QString DownloadThread::flv_host = "flv.nhk.or.jp";
QString DownloadThread::flv_app = "ondemand/";
QString DownloadThread::flv_service_prefix = "mp4:flv/gogaku/streaming/mp4/";

QString DownloadThread::flvstreamer;
QString DownloadThread::ffmpeg;
QString DownloadThread::openssl;
QString DownloadThread::scramble;
QStringList DownloadThread::malformed = (QStringList() << "3g2" << "3gp" << "m4a" << "mov");

QString DownloadThread::getMasterM3u8( QString file ) {
	static QString master_m3u8_prefix = "https://nhk-vh.akamaihd.net/i/gogaku-stream/mp4/";
	static QString master_m3u8_suffix = "/master.m3u8";
	
	UrlDownloader urldownloader;
	QString temp = master_m3u8_prefix + file + master_m3u8_suffix;
	urldownloader.doDownload( master_m3u8_prefix + file + master_m3u8_suffix );
	//qDebug() << urldownloader.contents().constData();
	return urldownloader.contents().length() ? QString::fromAscii(  urldownloader.contents().constData() ) : "";
}

QString DownloadThread::getIndexM3u8( QString masterM3u8 ) {
	static QRegExp regexp( "http[^\n]*" );
	
	QString result;
	if ( regexp.indexIn( masterM3u8, 0 ) != -1 ) {
		QString indexUrl = regexp.cap( 0 );
		UrlDownloader urldownloader;
		urldownloader.doDownload( indexUrl );
		//qDebug() << QByteArray::fromPercentEncoding( urldownloader.contents() ).constData();
		if ( urldownloader.contents().length() )
			result = QString::fromAscii(  QByteArray::fromPercentEncoding( urldownloader.contents() ).constData() );
	}
	return result;
}

QByteArray DownloadThread::getCryptKey( QString indexM3u8 ) {
	static QRegExp regexp( "#EXT-X-KEY:METHOD=AES-128,URI=\"([^\"]*)\"" );
	
	QByteArray result;
	if ( regexp.indexIn( indexM3u8, 0 ) != -1 ) {
		QString cryptKeyUri = QString::fromAscii( QByteArray::fromPercentEncoding( regexp.cap( 1 ).toAscii() ).constData() );
		//qDebug() << cryptKeyUri;
		UrlDownloader urldownloader;
		urldownloader.doDownload( cryptKeyUri );
		if ( urldownloader.contents().length() )
			result = urldownloader.contents().toHex();
	}
	return result;
}

QStringList DownloadThread::getSegmentUrlList( QString indexM3u8 ) {
	static QRegExp regexp( "http:[^\n]*" );
	
	QStringList result;
	for ( int from = 0; ( from = regexp.indexIn( indexM3u8, from ) ) != -1; from++ ) {
		QString segmentUrl = QString::fromAscii( QByteArray::fromPercentEncoding( regexp.cap( 0 ).toAscii() ).constData() );
		//qDebug() << segmentUrl;
		result << segmentUrl;
	}
	return result;
}

QString DownloadThread::downloadSegment( QString outputDir, QString segmentUrl ) {
	static QRegExp regexp( "http.*/mp4/(.*).mp4/([^\?]*)\?" );
	QString result;
	
	if ( regexp.indexIn( segmentUrl, 0 ) == -1 ) {
		emit critical( QString::fromUtf8( "セグメントファイルのURLが認識できません：　" ) + segmentUrl );
		return result;
	}
	QString segmentName = regexp.cap( 1 ) + "-" + regexp.cap( 2 );
	//qDebug() << segmentName;
	
	UrlDownloader urldownloader;
	urldownloader.doDownload( segmentUrl );
	int segmentLength = urldownloader.contents().length();
	//qDebug() << segmentLength;
	if ( !segmentLength ) {
		emit critical( QString::fromUtf8( "セグメントのダウンロードに失敗しました：　" ) + segmentName );
	} else {
		QFile segmentFile( outputDir + segmentName );
		if ( !segmentFile.open( QIODevice::WriteOnly ) ) {
			emit critical( QString::fromUtf8( "セグメントファイルの作成に失敗しました：　" ) + segmentName );
		} else {
			if ( segmentFile.write( urldownloader.contents() ) != segmentLength )
				emit critical( QString::fromUtf8( "セグメントファイルの書き込みに失敗しました：　" ) + segmentName );
			else
				result = segmentName;
			segmentFile.close();
		}
	}
	return result;
}

bool DownloadThread::decryptSegment( QString outputDir, QString segmentName, int index, QString cryptKey ) {
	bool result = false;
	QString encrypted = outputDir + segmentName;
	QString derypted = outputDir + segmentName + "_d";
	QString commandOpenssl = QString( "\"%1\" %2 -d -in \"%3\" -out \"%4\" -nosalt -iv %5 -K %6" )
			.arg( openssl, "aes-128-cbc", encrypted, derypted,
				  QString( "%1").arg( index, 32, 16, QLatin1Char( '0' ) ), cryptKey );
	//qDebug() << commandOpenssl;
	QProcess process;
	if ( process.execute( commandOpenssl ) ) {
		emit critical( QString::fromUtf8( "復号化に失敗しました：　" ) + encrypted );
	} else {
		if ( !QFile::remove( encrypted ) )
			emit critical( QString::fromUtf8( "ファイルの削除に失敗しました：　" ) + encrypted );
		else if ( !QFile::rename( derypted, encrypted ) )
			emit critical( QString::fromUtf8( "ファイル名の変更に失敗しました：　" ) + derypted );
		else
			result = true;
	}
	return result;
}

bool DownloadThread::mergeSegments( QString outputDir, QStringList segmentNames, QString tsName ) {
	bool result = false;
	QFile tsFile( outputDir + tsName );
	if ( !tsFile.open( QIODevice::WriteOnly ) )
		emit critical( OriginalFormat + QString::fromUtf8( "ファイルの作成に失敗しました：　" ) + tsName );
	else {
		int i;
		for ( i = 0; i < segmentNames.count(); i++ ) {
			QFile segmentFile( outputDir + segmentNames[i] );
			if ( !segmentFile.open( QIODevice::ReadOnly ) ) {
				emit critical( QString::fromUtf8( "セグメントファイルのオープンに失敗しました：　" ) + segmentNames[i] );
				segmentFile.close();
				break;
			}
			if ( tsFile.write( segmentFile.readAll() ) != segmentFile.size() ) {
				emit critical( OriginalFormat + QString::fromUtf8( "ファイルの書き込みに失敗しました：　" ) + tsName );
				segmentFile.close();
				break;
			}
		}
		if ( i >= segmentNames.count() )
			result = true;
		tsFile.close();
	}
	if ( !result && tsFile.exists() )
		tsFile.remove();
	return result;
}

bool DownloadThread::convertFormat( QString outputDir, QString tsName, QString outBasename, QString extension, QString id3tagTitle, QString kouza, QString hdate, QString file ) {
	int month = hdate.left( 2 ).toInt();
	int year = 2000 + file.left( 2 ).toInt();
	if ( month <= 4 && QDate::currentDate().year() > year )
		year += 1;
	int day = hdate.mid( 3, 2 ).toInt();
	QDate onair( year, month, day );
	QString yyyymmdd = onair.toString( "yyyy_MM_dd" );
	QString outFileName = outBasename + "." + extension;
	QString srcPath = outputDir + tsName;
	QString dstPath = outputDir + outFileName;
	QFileInfo fileInfo( srcPath );
	bool result = false;
	
	if ( !isCanceled ) {
		QString commandFfmpeg = extension == "mp3" ?
				QString( "\"%1\" -i \"%2\" -vn -acodec libmp3lame -ar 22050 -ac 1 -ab 48k -y \"%3\"" )
						.arg( ffmpeg, srcPath, dstPath ) :
				QString( "\"%1\" -i \"%2\" %3 -vn -acodec copy -y \"%4\"" )
					.arg( ffmpeg, srcPath, malformed.contains( extension ) ? FilterOption : "", dstPath );
		qDebug() << commandFfmpeg;
		emit current( extension + QString::fromUtf8( "へ変換中：　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
		QProcess process;
		if ( process.execute( commandFfmpeg ) ) {
			emit critical( extension + QString::fromUtf8( "への変換を完了できませんでした：　" ) +
					kouza + QString::fromUtf8( "　" ) + yyyymmdd );
		} else {
			if ( extension == "mp3" ) {
				QString error;
				if ( !MP3::id3tag( dstPath, kouza, id3tagTitle, QString::number( year ), "NHK", error ) )
					emit critical( error );
				else
					result = true;
			} else
				result = true;
		}
	}
	if ( QFile::exists( srcPath ) && ( extension != OriginalFormat || fileInfo.size() <= FLV_MIN_SIZE ) )
		QFile::remove( srcPath );
	return result;
}

bool DownloadThread::captureStream( QString kouza, QString hdate, QString file ) {
	QString outputDir = MainWindow::outputDir + kouza;
	if ( !checkOutputDir( outputDir ) )
		return false;
	outputDir += QDir::separator();	//通常ファイルが存在する場合のチェックのために後から追加する

	QString titleFormat;
	QString fileNameFormat;
	CustomizeDialog::formats( kouza, titleFormat, fileNameFormat );
	QString id3tagTitle = formatName( titleFormat, kouza, hdate, file, false );
	QString outFileName = formatName( fileNameFormat, kouza, hdate, file, true );
	QFileInfo fileInfo( outFileName );
	QString outBasename = fileInfo.completeBaseName();
	
	// 2013/04/05 オーディオフォーマットの変更に伴って拡張子の指定に対応
	QString extension = ui->comboBox_extension->currentText();
	outFileName = outBasename + "." + extension;

#ifdef Q_WS_WIN
	QString null( "nul" );
#else
	QString null( "/dev/null" );
#endif

	int month = hdate.left( 2 ).toInt();
	int year = 2000 + file.left( 2 ).toInt();
	if ( month <= 4 && QDate::currentDate().year() > year )
		year += 1;
	int day = hdate.mid( 3, 2 ).toInt();
	QDate onair( year, month, day );
	QString yyyymmdd = onair.toString( "yyyy_MM_dd" );
	
	if ( ui->checkBox_skip->isChecked() && QFile::exists( outputDir + outFileName ) ) {
		emit current( QString::fromUtf8( "スキップ：　　　　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
		return true;
	}
	emit current( QString::fromUtf8( "ダウンロード中：　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
	
	QString masterM3u8 = getMasterM3u8( file );
	if ( !masterM3u8.length() ) {
		emit critical( QString::fromUtf8( "master.m3u8の取得に失敗しました：　" ) +
				kouza + QString::fromUtf8( "　" ) + hdate );
		return false;
	}
	QString indexM3u8 = getIndexM3u8( masterM3u8 );
	if ( !indexM3u8.length() ) {
		emit critical( QString::fromUtf8( "index_0_a.m3u8の取得に失敗しました：　" ) +
				kouza + QString::fromUtf8( "　" ) + hdate );
		return false;
	}
	QByteArray cryptKey = getCryptKey( indexM3u8 );
	if ( !cryptKey.length() ) {
		emit critical( QString::fromUtf8( "crypt.keyの取得に失敗しました：　" ) +
				kouza + QString::fromUtf8( "　" ) + hdate );
		return false;
	}
	QStringList segmentUrlList = getSegmentUrlList( indexM3u8 );
	if ( segmentUrlList.isEmpty() ) {
		emit critical( QString::fromUtf8( "音声ファイルセグメントのURLの取得に失敗しました：　" ) +
				kouza + QString::fromUtf8( "　" ) + hdate );
		return false;
	}
	
	QStringList::const_iterator constIterator;
	int index = 1;
	QStringList segmentNames;
	for ( constIterator = segmentUrlList.constBegin(); constIterator != segmentUrlList.constEnd() && !isCanceled; constIterator++ ) {
        //qDebug() << *constIterator;
		QString segmentName = downloadSegment( outputDir, *constIterator );
		if ( !segmentName.length() )
			break;
		segmentNames << segmentName;
		if ( !decryptSegment( outputDir, segmentName, index, cryptKey ) )
			break;
		index++;
	}
	if ( constIterator == segmentUrlList.constEnd() ) {
		mergeSegments( outputDir, segmentNames, outBasename + "." + OriginalFormat );
		if ( extension != OriginalFormat )
			convertFormat( outputDir, outBasename + "." + OriginalFormat, outBasename, extension, id3tagTitle, kouza, hdate, file );
	}
	for ( int i = 0; i < segmentNames.count(); i++ )
		QFile::remove( outputDir + segmentNames[i] );
	
	return constIterator == segmentUrlList.constEnd();
}

QString DownloadThread::paths[] = {
	"english/basic1", "english/basic2", "english/basic3", "english/timetrial",
	"english/kaiwa", "english/business1", "english/business2",
	"chinese/kouza", "french/kouza", "italian/kouza", "hangeul/kouza",
	"german/kouza", "spanish/kouza", "english/kouryaku", "english/yomu",
	"chinese/levelup", "hangeul/levelup", "russian/kouza"
};

void DownloadThread::run() {
	QCheckBox* checkbox[] = {
		ui->checkBox_0, ui->checkBox_1, ui->checkBox_2, ui->checkBox_3, ui->checkBox_4,
		ui->checkBox_5, ui->checkBox_6, ui->checkBox_7, ui->checkBox_8, ui->checkBox_9,
		ui->checkBox_10, ui->checkBox_11, ui->checkBox_12, ui->checkBox_kouryaku, ui->checkBox_yomu,
		ui->checkBox_16, ui->checkBox_17, ui->checkBox_18, NULL
	};

	if ( !isOpensslAvailable( openssl ) || !isFfmpegAvailable( ffmpeg ) )
		return;

	emit information( QString::fromUtf8( "2013年7月29日対応版です。" ) );
	emit information( QString::fromUtf8( "ニュースで英会話とABCニュースシャワーは未対応です。" ) );
	emit information( QString::fromUtf8( "----------------------------------------" ) );

	for ( int i = 0; checkbox[i] && !isCanceled; i++ ) {
		if ( checkbox[i]->isChecked() ) {
			QStringList fileList = getAttribute( prefix + paths[i] + "/" + suffix, "@file" );
			QStringList kouzaList = getAttribute( prefix + paths[i] + "/" + suffix, "@kouza" );
			QStringList hdateList = one2two( getAttribute( prefix + paths[i] + "/" + suffix, "@hdate" ) );

			if ( fileList.count() && fileList.count() == kouzaList.count() && fileList.count() == hdateList.count() ) {
				if ( true /*ui->checkBox_this_week->isChecked()*/ ) {
					for ( int j = 0; j < fileList.count() && !isCanceled; j++ )
						captureStream( kouzaList[j], hdateList[j], fileList[j] );
				}
			}
		}
	}
	
	//if ( !isCanceled && ui->checkBox_shower->isChecked() )
		//downloadShower();

	//if ( !isCanceled && ui->checkBox_14->isChecked() )
		//downloadENews( false );

	//if ( !isCanceled && ui->checkBox_15->isChecked() )
		//downloadENews( true );

	emit current( "" );
	//キャンセル時にはdisconnectされているのでemitしても何も起こらない
	emit information( QString::fromUtf8( "ダウンロード作業が終了しました。" ) );
}
