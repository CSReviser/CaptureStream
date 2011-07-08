#include "utility.h"
#include "urldownloader.h"

#include <QUrl>
#include <QRegExp>
#include <QCoreApplication>
#include <QDir>
#include <QTemporaryFile>
#include <QProcess>
#include <QFileInfo>
#include <QFile>
#include <QByteArray>
#include <QXmlQuery>
#include <QScriptEngine>
#include <QScriptValue>
#include <QDebug>
#include <QDateTime>

namespace {
	const QString UPUPUP( "/../../.." );
	const QString FLARE( "flare" );
	const QString GNASH( "gnash -r0 -v http://www.nhk.or.jp/gogaku/common/swf/streaming.swf" );
	const QUrl STREAMINGSWF( "http://www.nhk.or.jp/gogaku/common/swf/streaming.swf" );
	const QString TEMPLATE( "streamingXXXXXX.swf" );

	const QRegExp REGEXP( "function startInit\\(\\) \\{[^}]*\\}\\s*function (\\w*).*startInit\\(\\);" );
	const QRegExp TAIL( "CONNECT_DIRECTORY \\+ '(.*)/' \\+ INIT_URI" );

	const QString LISTDATAFLV( "http://www.nhk.or.jp/gogaku/common/swf/(\\w+)/listdataflv.xml" );
	const QString WIKIXML2( "doc('http://www47.atwiki.jp/jakago/pub/scramble.xml')/flv/scramble[@date=\"" );
	const QString WIKIXML3( "\"]/@code/string()" );
}

// Macの場合はアプリケーションバンドル、それ以外はアプリケーションが含まれるディレクトリを返す
QString Utility::applicationBundlePath() {
	QString result = QCoreApplication::applicationDirPath();
#ifdef Q_WS_MAC
	result = QDir::cleanPath( result + UPUPUP );
#endif
	result += QDir::separator();
	return result;
}

// flareの出力を利用してスクランブル文字列を解析する
QString Utility::flare( QString& error ) {
	QString result;
	UrlDownloader urldownloader;
	urldownloader.doDownload( STREAMINGSWF );

	if ( urldownloader.contents().length() ) {
		QTemporaryFile file;
		file.setFileTemplate( QDir::tempPath() + QDir::separator() + TEMPLATE );
		if ( file.open() ) {
			file.write( urldownloader.contents() );
			file.close();
			QProcess process;
			process.start( applicationBundlePath() + FLARE, QStringList( file.fileName() ) );
			if ( process.waitForStarted() && process.waitForFinished() ) {
				QFileInfo info( file.fileName() );
				QString flr = info.absolutePath() + QDir::separator() + info.completeBaseName() + ".flr";
				QFile scriptFile( flr );
				if ( scriptFile.open( QIODevice::ReadOnly ) ) {
					QByteArray bytes = scriptFile.readAll();
					QString contents = QString::fromUtf8( bytes.constData() );
					scriptFile.close();
					if ( REGEXP.indexIn( contents, 0 ) != -1 )
						contents = REGEXP.cap();
					QString tail;
					if ( TAIL.indexIn( contents, 0 ) != -1 )
						tail = TAIL.cap( 1 );
					QScriptEngine myEngine;
					myEngine.evaluate( contents );
					QScriptValue generator = myEngine.globalObject().property( REGEXP.cap( 1 ) );
					if ( !generator.isValid() ) {
						contents.replace( ".", " ." );
						myEngine.evaluate( contents );
						generator = myEngine.globalObject().property( REGEXP.cap( 1 ) );
					}
					if ( generator.isValid() ) {
						generator.call();
						QRegExp variable( "(\\w+)[^\\n=]*=[^\\n]*\\n[^\\n]*\\}" );
						if ( variable.indexIn( generator.toString(), 0 ) != -1 )
							result = myEngine.globalObject().property( variable.cap( 1 ) ).toString() + tail;
					}
				}
				QFile::remove( flr );
				if ( !result.length() )
					error = QString::fromUtf8( "コードの取得に失敗しました。" );
			} else
				error = QString::fromUtf8( "flareが存在しないか実行に失敗しました。" );
		}
	}
	return result;
}

// gnashの出力を利用してスクランブル文字列を解析する
QString Utility::gnash( QString& error ) {
	QString result;
	QProcess process;
	process.start( GNASH );
	bool started = process.waitForStarted();
	if ( !started ) {
		process.start( "sdl-" + GNASH );
		started = process.waitForStarted();
	}
	if ( started ) {
		QRegExp regexp( LISTDATAFLV );
		while ( process.waitForReadyRead( 5000 ) ) {
			QByteArray bytes = process.readAllStandardOutput();
			QString output = QString::fromUtf8( bytes.data() );
			if ( regexp.indexIn( output ) != -1 ) {
				result = regexp.cap(1);
				process.terminate();
				break;
			}
		}
		if ( !result.length() )
			error = QString::fromUtf8( "コードの取得に失敗しました。" );
	} else
		error = QString::fromUtf8( "gnashが存在しないか実行に失敗しました。" );
	return result;
}

// ウィキからスクランブル文字列を取得する
QString Utility::wiki() {
	QDate today = QDate::currentDate();
	int offset = 1 - today.dayOfWeek();		//直前の月曜までのオフセット。月曜日なら0
	if ( offset == 0 && QTime::currentTime().hour() <= 9 )	//月曜日で10時より前なら1週間前の月曜日に
		offset = -7;
	QDate monday = today.addDays( offset );

	QString result;
	QStringList attributeList;
	QXmlQuery query;
	query.setQuery( WIKIXML2 + monday.toString( "yyyyMMdd" ) + WIKIXML3 );
	if ( query.isValid() ) {
		query.evaluateTo( &attributeList );
		if ( attributeList.count() > 0 )
			result = attributeList[0];
	}
	return result;
}
