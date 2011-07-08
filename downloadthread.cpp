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

#define FLVSTREAMER "flvstreamer"
#define Timeout " -m 10 "
#define ScrambleLength 14

DownloadThread::DownloadThread( Ui::MainWindowClass* ui ) : isCanceled(false) {
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

bool DownloadThread::checkFlvstreamer( QString& path ) {
	path = Utility::applicationBundlePath() + FLVSTREAMER;
#ifdef Q_WS_WIN
	path += ".exe";
#endif

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

QString mkTempName( QString original ) {
	QString result;

	for ( int i = 0; i < 100; i++ ) {
		QString now = QDateTime::currentDateTime().toString( "yyyyMMddhhmmsszzz" );
		if ( !QFile::exists( original + now ) ) {
			result = original + now;
			break;
		}
	}

	return result;
}

void DownloadThread::id3tag( QString fullPath, QString album, QString title, QString year, QString artist ) {
	QTextCodec* utf16 = QTextCodec::codecForName( "UTF-16" );
	if ( !utf16 ) {
		emit information( QString::fromUtf8( "文字コードの変換ができないのでID3v1の書き込みを中止します。" ) );
		return;
	}

	QByteArray tagBytes;
	MP3::createTag( tagBytes, album, title, year, artist, utf16 );
	if ( !tagBytes.size() ) {
		emit critical( QString::fromUtf8( "書き込むべきタグが見当たらないため、タグの書き込みを中止します。" ) );
		return;
	}

	QString tempName = mkTempName( fullPath );
	if ( !tempName.size() ) {
		emit critical( QString::fromUtf8( "作業用ファイル名が作成できないため、タグの書き込みを中止します。" ) );
		return;
	}

	if ( !QFile::rename( fullPath, tempName ) ) {
			emit critical( QString::fromUtf8( "ダウンロードしたMP3のファイル名が変更できないため、タグの書き込みを中止します。" ) );
			return;
	}

	QFile srcFile( tempName );
	if ( !srcFile.open( QIODevice::ReadOnly ) ) {
		emit critical( QString::fromUtf8( "ダウンロードしたMP3ファイルを開けないため、タグの書き込みを中止します。" ) );
		return;
	}

	QFile dstFile( fullPath );
	if ( !dstFile.open( QIODevice::WriteOnly ) ) {
		emit critical( QString::fromUtf8( "作業用ファイルの作成に失敗したため、タグの書き込みを中止します。Code:" ) +
					   QString::number( dstFile.error() ) + " Description:" + dstFile.errorString() );
		srcFile.close();
		return;
	}

	qint64 writtenSize = dstFile.write( tagBytes );
	if ( writtenSize != tagBytes.size() ) {
		dstFile.remove();
		srcFile.rename( fullPath );
		emit critical( QString::fromUtf8( "作業用ファイルへの書き込みに失敗しました。" ) );
		return;
	}

	QByteArray buffer = srcFile.readAll();
	srcFile.close();
	long skip = MP3::tagSize( buffer );
	writtenSize = dstFile.write( buffer.constData() + skip, buffer.size() - skip );
	dstFile.close();
	if ( writtenSize != buffer.size() - skip ) {
		dstFile.remove();
		srcFile.rename( fullPath );
		emit critical( QString::fromUtf8( "作業用ファイルへの書き込みに失敗しました。" ) );
		return;
	}

	if ( !srcFile.remove() )
		emit critical( QString::fromUtf8( "「" ) + tempName + QString::fromUtf8( "」の削除に失敗しました。" ) );
}

//--------------------------------------------------------------------------------

struct FlvHeader {
	unsigned char signature[3];
	unsigned char version;
	unsigned char flags;
	unsigned char offset[4];
};

struct FlvTag {
	unsigned char previousTagSize[4];
	unsigned char type;
	unsigned char bodyLength[3];
	unsigned char timestamp[3];
	unsigned char timestampExtended;
	unsigned char streamId[3];
	//このあとにbodyLengthのデータが続く
};

bool DownloadThread::flv2mp3( const QString& flvPath, const QString& mp3Path ) {
	bool result = false;

	try {
		QFile flv( flvPath );
		if ( !flv.open( QIODevice::ReadOnly ) ) {
			throw QString::fromUtf8( "flvファイルのオープンに失敗しました。Code:" ) +
					QString::number( flv.error() ) + " Description:" + flv.errorString();
		}

		QByteArray buffer = flv.readAll();
		flv.close();
		long bufferSize = buffer.length();

		if ( bufferSize < (long)sizeof (FlvHeader) )
			throw QString::fromUtf8( "flvファイルにヘッダが含まれていません。" );

		FlvHeader& header = *(FlvHeader*)buffer.constData();
		if ( strncmp( (const char*)header.signature, "FLV", sizeof header.signature ) )
			throw QString::fromUtf8( "flvファイルではありません。" );

		if ( (header.flags & 4) == 0 )
			throw QString::fromUtf8( "音声データが含まれていません。" );

		if ( header.offset[0] || header.offset[1] || header.offset[2] || header.offset[3] != sizeof header )
			throw QString::fromUtf8( "flvファイルが対応できる形式ではありません。" );

		long readSize = sizeof (FlvHeader);

		QFile mp3( mp3Path );
		if ( !mp3.open( QIODevice::WriteOnly ) ) {
			throw QString::fromUtf8( "mp3ファイルのオープンに失敗しました。Code:" ) +
					QString::number( mp3.error() ) + " Description:" + mp3.errorString();
		}

		const char* byte = buffer.constData();
		while ( true ) {
			if ( bufferSize - readSize == 4 )	//最後のPreviousTagSize
				throw true;
			if ( bufferSize - readSize < (long)sizeof (FlvTag) )
				throw QString::fromUtf8( "flvファイルの内容が不正です。" );
			FlvTag& tag = *(FlvTag*)(byte + readSize);
			readSize += sizeof (FlvTag);
			long bodyLength = (tag.bodyLength[0] << 16) + (tag.bodyLength[1] << 8) + tag.bodyLength[2];
			if ( bufferSize - readSize < bodyLength )
				throw QString::fromUtf8( "flvファイルの内容が不正です。" );
			if ( tag.type == 0x08 ) {
				if ( (byte[readSize] & 0x00f0) != 0x20 )
					throw QString::fromUtf8( "音声データがmp3ではありません。" );
				if ( mp3.write( byte + readSize + 1, bodyLength - 1 ) != bodyLength - 1 )
					throw QString::fromUtf8( "mp3ファイルの書き込みに失敗しました。" );
			}
			readSize += bodyLength;
		}
		mp3.close();
	} catch ( bool ) {
		//QFile::remove( flvPath );
		result = true;
	} catch ( QString& message ) {
		QFile::remove( mp3Path );
		emit critical( message );
	}

	return result;
}

//--------------------------------------------------------------------------------

void DownloadThread::downloadCharo() {
	QString flvstreamer;
	if ( !checkFlvstreamer( flvstreamer ) )
		return;
	QString kouza = QString::fromUtf8( "リトル・チャロ2" );
	QString outputDir = MainWindow::outputDir + kouza;
	if ( !checkOutputDir( outputDir ) )
		return;
	outputDir += QDir::separator();	//通常ファイルが存在する場合のチェックのために後から追加する
	QString flv_host( "flv9.nhk.or.jp");
	QString flv_app( "flv9/_definst_/" );
	QString flv_service_prefix( "flv:charo/streams/radio/" );
#ifdef Q_WS_WIN
	QString null( "nul" );
#else
	QString null( "/dev/null" );
#endif
	bool skip = ui->checkBox_skip->isChecked();

	QDate today = QDate::currentDate();
	int offset = 8 - today.dayOfWeek();	//次の月曜までのオフセット
	QDate monday = today.addDays( offset );
	QDate from = monday.addDays( -14 );	//ダウンロード可能な一番古い日付
	QDate start( 2010, 3, 29 );
	if ( from < start )
		from = start;

	for ( QDate i = from; i <= today && !isCanceled; i = i.addDays( 1 ) ) {
		if ( i.dayOfWeek() < Qt::Saturday ) {	//2010年度は月〜金
			QString hdate = i.toString( "yyyy_MM_dd" );
			QString flv_file = outputDir + kouza + "_" + hdate + ".flv";
			QString mp3_file = outputDir + kouza + "_" + hdate + ".mp3";
			QString server_file = i.toString( "yyyyMMdd" ) + ".flv";

			if ( skip && QFile::exists( mp3_file ) )
				emit current( QString::fromUtf8( "スキップ：　　　　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
			else {
				QString command1935 = "\"" + flvstreamer + "\"" + Timeout + " -r \"rtmp://" + flv_host + "/" + flv_app +
						flv_service_prefix + server_file + "\" -o " + "\"" + flv_file + "\" > " + null;
				QString command80 = "\"" + flvstreamer + "\"" + Timeout + " -r \"rtmpt://" + flv_host + ":80/" + flv_app +
						flv_service_prefix + server_file + "\" -o " + "\"" + flv_file + "\" --resume > " + null;
				QProcess process;
				emit current( QString::fromUtf8( "ダウンロード中：　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
				int exitCode = process.execute( command1935 );
				if ( exitCode && !isCanceled )
					exitCode = process.execute( command80 );

				bool keep_on_error = ui->checkBox_keep_on_error->isChecked();
				if ( exitCode && !isCanceled ) {
					QFile flv( flv_file );
					if ( flv.size() > 0 ) {
						for ( int count = 0; exitCode && count < 5 && !isCanceled; count++ ) {
							emit current( QString::fromUtf8( "リトライ中：　　　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
							exitCode = process.execute( command80 );
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
					if ( flv2mp3( flv_file, mp3_file ) )
						id3tag( mp3_file, kouza, kouza + "_" + hdate, i.toString( "yyyy" ), "NHK" );
				}
				if ( QFile::exists( flv_file ) )
					QFile::remove( flv_file );
			}
		}
	}
}

//--------------------------------------------------------------------------------

void DownloadThread::downloadENews( bool re_read ) {
	emit current( QString::fromUtf8( "ニュースで英会話のhtmlを分析中" ) );
	DownloadManager manager( re_read );
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
		//foreach( QString flv, list ) {
			if ( isCanceled )
				break;
			int year = flv.mid( 0, 4 ).toInt();
			int month = flv.mid( 4, 2 ).toInt();
			int day = flv.mid( 7, 2 ).toInt();
			QDate theDay( year, month, day );
			//QString flv_service_prefix = year > 2009 || ( year == 2009 && month >= 8 ) || ( year == 2009 && month == 7 && day >= 28 ) ?
										 //flv_service_prefix_20090728 : flv_service_prefix_20090330;
			QString flv_service_prefix = theDay >= QDate( 2009, 7, 28 ) ? flv_service_prefix_20090728 : flv_service_prefix_20090330;
			//QString hdate = flv.mid( 4, 2 ) + QString::fromUtf8( "月" ) + flv.mid( 7, 2 ) + QString::fromUtf8( "日放送分" );
			QString hdate = theDay.toString( "yyyy_MM_dd" );
			int slashIndex = flv.lastIndexOf( '/' );
			QString flv_file = outputDir + ( slashIndex == -1 ? flv : flv.mid( slashIndex + 1 ) ) + ".flv";
			//QString flv_file = outputDir + kouza + "_" + hdate + ".flv";
			QString mp3_file = outputDir + kouza + "_" + hdate + ".mp3";
			QString movie_file = outputDir + kouza + "_" + hdate + ".flv";
			bool mp3Exists = QFile::exists( mp3_file );
			bool movieExists = QFile::exists( movie_file );

			if ( !skip || ( saveAudio && !mp3Exists ) || ( saveMovie && !movieExists ) ) {
				QString command1935 = "\"" + flvstreamer + "\"" + Timeout + " -r \"rtmp://" + flv_host + "/" + flv_app +
						flv_service_prefix + flv + "\" -o " + "\"" + flv_file + "\" > " + null;
				QString command80 = "\"" + flvstreamer + "\"" + Timeout + " -r \"rtmpt://" + flv_host + ":80/" + flv_app +
						flv_service_prefix + flv + "\" -o " + "\"" + flv_file + "\" --resume > " + null;
				QProcess process;
				emit current( QString::fromUtf8( "ダウンロード中：　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
				int exitCode = process.execute( command1935 );
				if ( exitCode && !isCanceled )
					exitCode = process.execute( command80 );

				bool keep_on_error = ui->checkBox_keep_on_error->isChecked();
				if ( exitCode && !isCanceled ) {
					QFile flv( flv_file );
					if ( flv.size() > 0 ) {
						for ( int count = 0; exitCode && count < 5 && !isCanceled; count++ ) {
							emit current( QString::fromUtf8( "リトライ中：　　　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
							exitCode = process.execute( command80 );
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
						//emit current( QString::fromUtf8( "音声抽出中：　　　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
						if ( flv2mp3( flv_file, mp3_file ) )
							id3tag( mp3_file, kouza, kouza + "_" + hdate, flv.left( 4 ), "NHK" );
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
	QString flv_host( "flv9.nhk.or.jp");
	QString flv_app( "flv9/_definst_/" );
	QString flv_service_prefix( "worldwave/common/movie/" );
#ifdef Q_WS_WIN
	QString null( "nul" );
#else
	QString null( "/dev/null" );
#endif
	bool skip = ui->checkBox_skip->isChecked();

	// 日本語ロケールではQDate::fromStringでMMMは動作しないのでQRegExpを使う
	// 日付の形式： Thu, 28 Apr 2011 00:00:00 +0900
	QStringList elements = getElements( "http://www.nhk.or.jp/worldwave/xml/abc_news.xml", "/rss/channel/item/pubDate/string()" );
	foreach (const QString &element, elements) {
		static QRegExp regexp( "^(?:[a-zA-Z]{3}), (\\d{1,2}) ([a-zA-Z]{3}) (\\d{4})" );
		if ( isCanceled )
			break;
		if ( regexp.indexIn( element ) != -1 ) {
			static QStringList months = QStringList()
					<< "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun"
					<< "Jul" << "Aug" << "Sep" << "Oct" << "Nov" << "Dec";
			qDebug() << regexp.cap( 0 );
			int year = regexp.cap( 3 ).toInt();
			int month = months.indexOf( regexp.cap( 2 ) ) + 1;
			int day = regexp.cap( 1 ).toInt();
			QDate date( year, month, day );
			qDebug() << date.toString( "yyyy_MM_dd" );
			QString hdate = date.toString( "yyyy_MM_dd" );
			QString flv_file = outputDir + kouza + "_" + hdate + ".flv";
			QString mp3_file = outputDir + kouza + "_" + hdate + ".mp3";
			QString server_file = "abc" + date.toString( "yyMMdd" ) + ".flv";
			bool flvExists = QFile::exists( flv_file );
			bool mp3Exists = QFile::exists( mp3_file );

			if ( !skip || ( saveAudio && !mp3Exists ) || ( saveMovie && !flvExists ) ) {
				QString command1935 = "\"" + flvstreamer + "\"" + Timeout + " -r \"rtmp://" + flv_host + "/" + flv_app +
						flv_service_prefix + server_file + "\" -o " + "\"" + flv_file + "\" > " + null;
				QString command80 = "\"" + flvstreamer + "\"" + Timeout + " -r \"rtmpt://" + flv_host + ":80/" + flv_app +
						flv_service_prefix + server_file + "\" -o " + "\"" + flv_file + "\" --resume > " + null;
				QProcess process;
				emit current( QString::fromUtf8( "ダウンロード中：　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
				qDebug() << command1935;
				int exitCode = process.execute( command1935 );
				if ( exitCode && !isCanceled )
					exitCode = process.execute( command80 );

				bool keep_on_error = ui->checkBox_keep_on_error->isChecked();
				if ( exitCode && !isCanceled ) {
					QFile flv( flv_file );
					if ( flv.size() > 0 ) {
						for ( int count = 0; exitCode && count < 5 && !isCanceled; count++ ) {
							emit current( QString::fromUtf8( "リトライ中：　　　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
							exitCode = process.execute( command80 );
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
						if ( flv2mp3( flv_file, mp3_file ) )
							id3tag( mp3_file, kouza, kouza + "_" + hdate, date.toString( "yyyy" ), "NHK" );
					}
				}
				if ( !saveMovie && QFile::exists( flv_file ) )
					QFile::remove( flv_file );
			} else
				emit current( QString::fromUtf8( "スキップ：　　　　" ) + kouza + QString::fromUtf8( "　" ) + hdate );
		}
	}
}

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
	if ( month <= 3 && QDate::currentDate().year() > year )
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

QString DownloadThread::paths[] = {
	"english/basic1", "english/basic2", "english/basic3", "english/training",
	"english/kaiwa", "english/business1", "english/business2",
	"chinese/kouza", "french/kouza", "italian/kouza", "hangeul/kouza",
	"german/kouza", "spanish/kouza"
};
QString DownloadThread::prefix = "http://www.nhk.or.jp/gogaku/";
QString DownloadThread::suffix = "/listdataflv.xml";

QString DownloadThread::flv_host = "flv9.nhk.or.jp";
QString DownloadThread::flv_app = "flv9/_definst_/";
QString DownloadThread::flv_service_prefix = "flv:gogaku/streaming/flv/";

QString DownloadThread::flvstreamer;
QString DownloadThread::scramble;

bool DownloadThread::captureStream( QString kouza, QString hdate, QString file, int retryCount, bool guess ) {
	QString outputDir = MainWindow::outputDir + kouza;
	if ( !checkOutputDir( outputDir ) )
		return false;
	outputDir += QDir::separator();	//通常ファイルが存在する場合のチェックのために後から追加する

#ifdef Q_WS_WIN
	QString null( "nul" );
#else
	QString null( "/dev/null" );
#endif

	QString titleFormat;
	QString fileNameFormat;
	CustomizeDialog::formats( kouza, titleFormat, fileNameFormat );
	QString id3tagTitle = formatName( titleFormat, kouza, hdate, file, false );
	QString outFileName = formatName( fileNameFormat, kouza, hdate, file, true );
	QFileInfo fileInfo( outFileName );
	QString outBasename = fileInfo.completeBaseName();

	bool result = false;

	if ( file.endsWith( ".flv", Qt::CaseInsensitive ) ) {
		int month = hdate.left( 2 ).toInt();
		int year = 2000 + file.left( 2 ).toInt();
		if ( month <= 3 && QDate::currentDate().year() > year )
			year += 1;
		int day = hdate.mid( 3, 2 ).toInt();
		QDate onair( year, month, day );
		QString yyyymmdd = onair.toString( "yyyy_MM_dd" );
		QString basename = file.left( file.size() - 4 );
		//QString outBasename = kouza + "_" + yyyymmdd;
		if ( ui->checkBox_skip->isChecked() && QFile::exists( outputDir + outFileName ) ) {
			emit current( QString::fromUtf8( "スキップ：　　　　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
			return true;
		}
		QString flv_file = outputDir + outBasename + ".flv";
		QString command1935 = "\"" + flvstreamer + "\"" + Timeout + " -r \"rtmp://" + flv_host + "/" + flv_app +
		flv_service_prefix + scramble + "/" + basename + "\" -o " + "\"" + flv_file + "\" > " + null;
		QString command80 = "\"" + flvstreamer + "\"" + Timeout + " -r \"rtmpt://" + flv_host + ":80/" + flv_app +
		flv_service_prefix + scramble + "/" + basename + "\" -o " + "\"" + flv_file + "\" --resume > " + null;
		QProcess process;
		if ( guess ) {
			//emit current( QString::fromUtf8( "別ファイル名でリトライ中．．．" ) );
			//emit messageWithoutBreak( QString::fromUtf8( "．" ) );
		} else
			emit current( QString::fromUtf8( "ダウンロード中：　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
		int exitCode = process.execute( command1935 );
		while ( exitCode && retryCount-- > 0 ) {
			emit current( QString::fromUtf8( "リトライ中：　　　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
			exitCode = process.execute( command80 );
		}
		if ( exitCode ) {
			//emit critical( QString::fromUtf8( "リトライしましたが、ダウンロードを完了できませんでした：　" ) +
			emit critical( QString::fromUtf8( "ダウンロードを完了できませんでした：　" ) +
						  kouza + QString::fromUtf8( "　" ) + yyyymmdd );
		}
		QFileInfo fileInfo( flv_file );	// ストリーミングが存在しなかった場合は13バイト
		if ( fileInfo.size() > 100 && ( !exitCode || ui->checkBox_keep_on_error->isChecked() ) ) {
			//emit current( QString::fromUtf8( "音声抽出中：　　　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
			if ( flv2mp3( flv_file, outputDir + outFileName ) ) {
				//id3tag( outputDir + outBasename + ".mp3", kouza, kouza + "_" + yyyymmdd, QString::number( year ), "NHK" );
				id3tag( outputDir + outFileName, kouza, id3tagTitle, QString::number( year ), "NHK" );
				result = true;
			}
		}
		if ( QFile::exists( flv_file ) )
			QFile::remove( flv_file );
	}

	return result;
}

bool DownloadThread::captureStreamPast( QString kouza, QString file, int retryCount, bool guess ) {
	Q_UNUSED( guess )
	QString hdate = QString::fromUtf8( "00月00日放送分" );
	QString outputDir = MainWindow::outputDir + kouza;
	if ( !checkOutputDir( outputDir ) )
		return false;
	outputDir += QDir::separator();	//通常ファイルが存在する場合のチェックのために後から追加する

#ifdef Q_WS_WIN
	QString null( "nul" );
#else
	QString null( "/dev/null" );
#endif

	//QString titleFormat;
	//QString fileNameFormat;
	//CustomizeDialog::formats( kouza, titleFormat, fileNameFormat );
	QString id3tagTitle = formatName( "%f", kouza, hdate, file, false );
	QString outFileName = formatName( "%f.mp3", kouza, hdate, file, true );
	QFileInfo fileInfo( outFileName );
	QString outBasename = fileInfo.completeBaseName();

	bool result = false;

	if ( file.endsWith( ".flv", Qt::CaseInsensitive ) ) {
		int month = hdate.left( 2 ).toInt();
		int year = 2000 + file.left( 2 ).toInt();
		if ( month <= 3 && QDate::currentDate().year() > year )
			year += 1;
		int day = hdate.mid( 3, 2 ).toInt();
		QDate onair( year, month, day );
		QString yyyymmdd = onair.toString( "yyyy_MM_dd" );
		QString basename = file.left( file.size() - 4 );
		//QString outBasename = kouza + "_" + yyyymmdd;
		if ( ui->checkBox_skip->isChecked() && QFile::exists( outputDir + outFileName ) ) {
			emit current( QString::fromUtf8( "スキップ：　　　　" ) + kouza + QString::fromUtf8( "　" ) + file );
			return true;
		}
		QString flv_file = outputDir + outBasename + ".flv";
		QString command1935 = "\"" + flvstreamer + "\"" + Timeout + " -r \"rtmp://" + flv_host + "/" + flv_app +
		flv_service_prefix + basename + "\" -o " + "\"" + flv_file + "\" > " + null;
		QString command80 = "\"" + flvstreamer + "\"" + Timeout + " -r \"rtmpt://" + flv_host + ":80/" + flv_app +
		flv_service_prefix + basename + "\" -o " + "\"" + flv_file + "\" --resume > " + null;
		QProcess process;
		//if ( guess ) {
			//emit current( QString::fromUtf8( "別ファイル名でリトライ中．．．" ) );
			//emit messageWithoutBreak( QString::fromUtf8( "．" ) );
		//} else
			emit current( QString::fromUtf8( "ダウンロード中：　" ) + kouza + QString::fromUtf8( "　" ) + file );
		int exitCode = process.execute( command1935 );
		while ( exitCode && retryCount-- > 0 ) {
			emit current( QString::fromUtf8( "リトライ中：　　　" ) + kouza + QString::fromUtf8( "　" ) + file );
			exitCode = process.execute( command80 );
		}
		if ( exitCode ) {
			//emit critical( QString::fromUtf8( "リトライしましたが、ダウンロードを完了できませんでした：　" ) +
			emit critical( QString::fromUtf8( "ダウンロードを完了できませんでした：　" ) +
						  kouza + QString::fromUtf8( "　" ) + yyyymmdd );
		}
		QFileInfo fileInfo( flv_file );	// ストリーミングが存在しなかった場合は13バイト
		if ( fileInfo.size() > 100 && ( !exitCode || ui->checkBox_keep_on_error->isChecked() ) ) {
			//emit current( QString::fromUtf8( "音声抽出中：　　　" ) + kouza + QString::fromUtf8( "　" ) + yyyymmdd );
			if ( flv2mp3( flv_file, outputDir + outFileName ) ) {
				//id3tag( outputDir + outBasename + ".mp3", kouza, kouza + "_" + yyyymmdd, QString::number( year ), "NHK" );
				id3tag( outputDir + outFileName, kouza, id3tagTitle, QString::number( year ), "NHK" );
				result = true;
			}
		}
		if ( QFile::exists( flv_file ) )
			QFile::remove( flv_file );
	}

	return result;
}

QDate onAirDate( QString hdate, QString file ) {
	int month = hdate.left( 2 ).toInt();
	int year = 2000 + file.left( 2 ).toInt();
	if ( month <= 3 && QDate::currentDate().year() > year )
		year += 1;
	int day = hdate.mid( 3, 2 ).toInt();
	return QDate( year, month, day );
}

static const int langCount = 13;
static int offsets[langCount][7] = {
	{ 5, 5, 5, 5, 5 },	//基礎英語１
	{ 5, 5, 5, 5, 5 },	//基礎英語２
	{ 5, 5, 5, 5, 5 },	//基礎英語３
	{ 7, 7, 7, 7, 7, 7, 7 },	//英語５分間トレーニング
	{ 5, 5, 5, 5, 5 },	//ラジオ英会話
	{ 2, 2 },			//入門ビジネス英語
	{ 3, 3, 3 },		//実践ビジネス英語
	{ 3, 3, 3, 2, 2 },	//まいにち中国語
	{ 3, 3, 3, 2, 2 },	//まいにちフランス語:2011/01/19修正
	{ 2, 2, 2, 2, 1 },	//まいにちイタリア語:2011/01/19修正
	{ 3, 3, 3, 2, 2 },	//まいにちハングル講座
	{ 4, 4, 4, 4, 1 },	//まいにちドイツ語
	{ 5, 5, 5, 5, 5 }	//まいにちスペイン語
};

void DownloadThread::downloadPast( int count, QString file, QString kouza ) {
	for ( int j = 1; j <= count && !isCanceled; j++ ) {
		QRegExp rx2( "^(\\d+-\\w+-\\d+-)(\\d+)([a-zA-Z]*)(?:[^-_.]*)(?:[^.]*)(.flv)$" );
		if ( rx2.indexIn( file, 0 ) != -1 ) {
			QString prefix = rx2.cap( 1 );
			QString number = rx2.cap( 2 );
			QString addition = rx2.cap( 3 );
			QString suffix = rx2.cap( 4 );

			QStringList tempList = (QStringList() << "" << "2" << "3" << "4");
			QStringList additions;
			additions << "";
			for ( int k = 0; k < tempList.count(); k++ ) {
				if ( addition.length() > 0 )
					additions << (addition + tempList[k]);
				if ( addition != "mm" )
					additions << ("mm" + tempList[k]);
				if ( addition != "vip" )
					additions << ("vip" + tempList[k]);
			}
			//#define Variants 6
			//static QString revisions[Variants] = { "", "-re01", "-re02", "_re01", "_re02", "-re" };
			//for ( int k = 0; k < Variants && !isCanceled; k++ ) {
			QStringList revisions = (QStringList() << "" << "-re01" << "-re02" << "_re01" << "_re02" << "-re");
			bool done = false;
			for ( int k = 0; k < revisions.count() && !isCanceled && !done; k++ ) {
				for ( int m = 0; m < additions.count() && !isCanceled && !done; m++ ) {
					QString file = QString( "%1%2%3%4%5" )
								   .arg( prefix ).arg( number.toInt() - j, 3, 10, QChar( '0' ) )
								   .arg( additions[m] ).arg( revisions[k] ).arg( suffix );
					//emit current( file );
					if ( captureStreamPast( kouza, file, 1, k || m ) )
						done = true;
					//else if ( !k && !m )
						//emit current( QString::fromUtf8( "別ファイル名でリトライ中\n" ) );
				}
			}
			if ( !done ) {
				//emit current( QString::fromUtf8( "" ) );
				//emit messageWithoutBreak( QString::fromUtf8( "サーバ上にファイルが存在しないか、ファイル名の推測に失敗しました。" ) );
				emit current( QString::fromUtf8( "サーバ上にファイルが存在しないか、ファイル名の推測に失敗しました。" ) );
			}
			//file = "#{prefix}#{sprintf( "%03d", number.to_i + offset[$target][count] )}#{revision}#{suffix}"
		}
	}
}

void DownloadThread::downloadOneWeek( int i, int addDays, QStringList& fileList, QStringList& kouzaList, QStringList& hdateList ) {
	for ( int j = 0; j < fileList.count() && !isCanceled; j++ ) {
		QDate onair = onAirDate( hdateList[j], fileList[j] );
		onair = onair.addDays( addDays );
		QRegExp rx( "^(?:\\d+)(\\D+)(?:\\d+)(\\D+)$" );
		if ( rx.indexIn( hdateList[j], 0 ) != -1 ) {
			QString hdate = QString( "%1%2%3%4" )
					.arg( onair.month(), 2, 10, QChar( '0' ) ).arg( rx.cap( 1 ) )
					.arg( (int)onair.day(), 2, 10, QChar( '0' ) ).arg( rx.cap( 2 ) );
			QRegExp rx2( "^(\\d+-\\w+-\\d+-)(\\d+)([a-zA-Z]*)(?:[^-_.]*)(?:[^.]*)(.flv)$" );
			if ( rx2.indexIn( fileList[j], 0 ) != -1 ) {
				QString prefix = rx2.cap( 1 );
				QString number = rx2.cap( 2 );
				QString addition = rx2.cap( 3 );
				QString suffix = rx2.cap( 4 );

				QStringList tempList = (QStringList() << "" << "2" << "3" << "4");
				QStringList additions;
				additions << "";
				for ( int k = 0; k < tempList.count(); k++ ) {
					if ( addition.length() > 0 )
						additions << (addition + tempList[k]);
					if ( addition != "mm" )
						additions << ("mm" + tempList[k]);
					if ( addition != "vip" )
						additions << ("vip" + tempList[k]);
				}
//#define Variants 6
				//static QString revisions[Variants] = { "", "-re01", "-re02", "_re01", "_re02", "-re" };
				//for ( int k = 0; k < Variants && !isCanceled; k++ ) {
				QStringList revisions = (QStringList() << "" << "-re01" << "-re02" << "_re01" << "_re02" << "-re");
				bool done = false;
				for ( int k = 0; k < revisions.count() && !isCanceled && !done; k++ ) {
					for ( int m = 0; m < additions.count() && !isCanceled && !done; m++ ) {
						QString file = QString( "%1%2%3%4%5" )
									   .arg( prefix ).arg( number.toInt() + offsets[i][j], 3, 10, QChar( '0' ) )
									   .arg( additions[m] ).arg( revisions[k] ).arg( suffix );
						if ( captureStream( kouzaList[j], hdate, file, 1, k || m ) )
							done = true;
						//else if ( !k && !m )
							//emit current( QString::fromUtf8( "別ファイル名でリトライ中\n" ) );
					}
				}
				if ( !done ) {
					//emit current( QString::fromUtf8( "" ) );
					//emit messageWithoutBreak( QString::fromUtf8( "サーバ上にファイルが存在しないか、ファイル名の推測に失敗しました。" ) );
					emit current( QString::fromUtf8( "サーバ上にファイルが存在しないか、ファイル名の推測に失敗しました。" ) );
				}
				//file = "#{prefix}#{sprintf( "%03d", number.to_i + offset[$target][count] )}#{revision}#{suffix}"
			}
		}
	}
}

void DownloadThread::run() {
	QCheckBox* checkbox[] = {
		ui->checkBox_0, ui->checkBox_1, ui->checkBox_2, ui->checkBox_3, ui->checkBox_4,
		ui->checkBox_5, ui->checkBox_6, ui->checkBox_7, ui->checkBox_8, ui->checkBox_9,
		ui->checkBox_10, ui->checkBox_11, ui->checkBox_12, NULL
	};

	if ( !checkFlvstreamer( flvstreamer ) )
		return;

	scramble = MainWindow::scramble;

	if ( scramble.length() )
		emit information( QString::fromUtf8( "ユーザ設定によるコード：" ) + scramble );

	if ( !scramble.length() ) {
		scramble = Utility::wiki();
		if ( scramble.length() )
			emit information( QString::fromUtf8( "wikiから取得したコード：" ) + scramble );
		else
			emit information( QString::fromUtf8( "wikiから取得したコード： 取得に失敗したか、まだwikiのxmlが更新されていません。" ) );
	}

	if ( !scramble.length() ) {
		QString error;
		scramble = Utility::gnash( error );
		if ( scramble.length() )
			emit information( QString::fromUtf8( "gnashを利用して解析したコード：" ) + scramble );
		else
			emit information( QString::fromUtf8( "gnashを利用して解析したコード： " ) + error );
	}

	if ( !scramble.length() ) {
		QString error;
		scramble = Utility::flare( error );
		if ( scramble.length() == ScrambleLength )
			emit information( QString::fromUtf8( "flareを利用して解析したコード：" ) + scramble );
		else
			emit information( QString::fromUtf8( "flareを利用して解析したコード：" ) + error );
	}

	if ( !scramble.length() ) {
		emit information( QString::fromUtf8( "スクランブル文字列が取得できなかったので終了します。" ) );
		return;
	}

	for ( int i = 0; checkbox[i] && !isCanceled; i++ ) {
		if ( checkbox[i]->isChecked() ) {
			QStringList fileList = getAttribute( prefix + paths[i] + "/" + scramble + suffix, "@file" );
			QStringList kouzaList = getAttribute( prefix + paths[i] + "/" + scramble + suffix, "@kouza" );
			QStringList hdateList = one2two( getAttribute( prefix + paths[i] + "/" + scramble + suffix, "@hdate" ) );

			if ( fileList.count() && fileList.count() == kouzaList.count() && fileList.count() == hdateList.count() ) {
				if ( true /*ui->checkBox_this_week->isChecked()*/ ) {
					for ( int j = 0; j < fileList.count() && !isCanceled; j++ )
						captureStream( kouzaList[j], hdateList[j], fileList[j], 5 );
				}

				if ( false /*ui->checkBox_past_week->isChecked()*/ ) {
					emit current( QString::fromUtf8( "＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊" ) );
					emit current( QString::fromUtf8( "「過去のストリーミング」は試験的に搭載された機能です。" ) );
					emit current( QString::fromUtf8( "＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊" ) );
					downloadPast( ui->past_days->text().toInt(), fileList[0], kouzaList[0] );
				}

				if ( false /*ui->checkBox_next_week->isChecked()*/ ) {
					emit current( QString::fromUtf8( "＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊" ) );
					emit current( QString::fromUtf8( "「次週のストリーミング」は試験的に搭載された機能です。" ) );
					emit current( QString::fromUtf8( "＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊" ) );
					//ゴールデンウィーク対応
					QString lastFile = fileList[fileList.count() - 1];
					QRegExp rx2( "^(\\d+-\\w+-\\d+-)(\\d+)([a-zA-Z]*)(?:[^-_.]*)(?:[^.]*)(.flv)$" );
					if ( rx2.indexIn( lastFile, 0 ) != -1 && rx2.cap( 3 ) == "gw" ) {
						QString prefix = rx2.cap( 1 );
						QString number = rx2.cap( 2 );
						QString suffix = rx2.cap( 4 );
						QString lastHdate = hdateList[hdateList.count() - 1];

						QDate onair = onAirDate( lastHdate, lastFile );
						onair = onair.addDays( -hdateList.count() + 1 );
						QRegExp rx( "^(?:\\d+)(\\D+)(?:\\d+)(\\D+)$" );
						if ( rx.indexIn( lastHdate, 0 ) != -1 ) {
							for ( int j = 0; j < hdateList.count(); j++ ) {
								hdateList[j] = QString( "%1%2%3%4" )
										.arg( onair.month(), 2, 10, QChar( '0' ) ).arg( rx.cap( 1 ) )
										.arg( (int)onair.day(), 2, 10, QChar( '0' ) ).arg( rx.cap( 2 ) );
								onair = onair.addDays( 1 );
								fileList[j] = QString( "%1%2%3" )
										   .arg( prefix ).arg( number.toInt() - hdateList.count() + j + 1, 3, 10, QChar( '0' ) )
										   .arg( suffix );
							}
						}
					}
					//ゴールデンウィークここまで

					downloadOneWeek( i, 7, fileList, kouzaList, hdateList );
				}
			}
		}
	}
	
	if ( !isCanceled && ui->checkBox_13->isChecked() )
		downloadCharo();

	if ( !isCanceled && ui->checkBox_14->isChecked() )
		downloadENews( false );

	if ( !isCanceled && ui->checkBox_15->isChecked() )
		downloadENews( true );

	if ( !isCanceled && ui->checkBox_shower->isChecked() )
		downloadShower();

	emit current( "" );
	//キャンセル時にはdisconnectされているのでemitしても何も起こらない
	emit information( QString::fromUtf8( "ダウンロード作業が終了しました。" ) );
}
