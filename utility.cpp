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
	//const QString GNASH( "gnash -r0 -t3 -v http://www.nhk.or.jp/gogaku/common/swf/streaming.swf" );
	const QString GNASH( "gnash -r0 -v http://www.nhk.or.jp/gogaku/common/swf/streaming.swf" );
	const QUrl STREAMINGSWF( "http://www.nhk.or.jp/gogaku/common/swf/streaming.swf" );
	const QString TEMPLATE( "streamingXXXXXX.swf" );

	//const QRegExp REGEXP( "function startInit.*startInit\\(\\);" );
	//const QRegExp REGEXP( "function\\s+startInit\\s*\\([^)]*\\)\\s*\\{[^}]*\\}\\s*function\\s+(\\w*)\\s*\\([^)]*\\)\\s*\\{\\s*(\\w+)\\s*=\\s*[^}]*\\}\\s*function\\s+\\w*\\s*\\([^)]*\\)\\s*\\{[^}]*\\}" );
	//const QRegExp REGEXP( "function\\s+startInit\\s*\\([^)]*\\)\\s*\\{[^}]*\\}\\s*function\\s+(\\w*)\\s*\\([^)]*\\)\\s*\\{\\s*(\\w+)\\s*=\\s*[^}]*\\}.*startInit\\s*\\(\\s*\\)\\s*;" );
	const QRegExp REGEXP( "function startInit\\(\\) \\{[^}]*\\}\\s*function (\\w*).*startInit\\(\\);" );
	const QRegExp TAIL( "CONNECT_DIRECTORY \\+ '(.*)/' \\+ INIT_URI" );

	//const QString GENERATOR( "setDirectory" );
	//const QString DIRECTORY( "CONNECT_DIRECTORY" );
	//const QString DIRECTORY( "str" );
	const QString LISTDATAFLV( "http://www.nhk.or.jp/gogaku/common/swf/(\\w+)/listdataflv.xml" );
	const QString WIKIXML( "doc('http://www47.atwiki.jp/jakago/pub/scramble.xml')/flv/scramble/@code/string()" );
	const QString WIKIXML2( "doc('http://www47.atwiki.jp/jakago/pub/scramble.xml')/flv/scramble[@date=\"" );
	const QString WIKIXML3( "\"]/@code/string()" );
}

// Macの場合はアプリケーションバンドルが含まれるディレクトリを返す
QString Utility::applicationDirPath() {
	QString applicationDirPath = QCoreApplication::applicationDirPath();
#ifdef Q_WS_MAC
	applicationDirPath = QDir::cleanPath( applicationDirPath + UPUPUP );
#endif
	applicationDirPath += QDir::separator();
	return applicationDirPath;
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
			//process.start( "\"" + applicationDirPath() + FLARE + "\" " + file.fileName() );
			process.start( applicationDirPath() + FLARE, QStringList( file.fileName() ) );
			if ( process.waitForStarted() && process.waitForFinished() ) {
				QFileInfo info( file.fileName() );
				QString flr = info.absolutePath() + QDir::separator() + info.completeBaseName() + ".flr";
				QFile scriptFile( flr );
				if ( scriptFile.open( QIODevice::ReadOnly ) ) {
					QByteArray bytes = scriptFile.readAll();
					QString contents = QString::fromUtf8( bytes.constData() );
					scriptFile.close();
					if ( REGEXP.indexIn( contents, 0 ) != -1 ) {
						contents = REGEXP.cap();
						//qDebug() << contents;
					}
					QString tail;
					if ( TAIL.indexIn( contents, 0 ) != -1 ) {
						tail = TAIL.cap( 1 );
						//qDebug() << TAIL.cap( 1 );
					}
					QScriptEngine myEngine;
					/*QScriptValue scriptValue =*/ myEngine.evaluate( contents );
					//QScriptValue generator = myEngine.globalObject().property( GENERATOR );
					//qDebug() << REGEXP.cap( 1 ) << REGEXP.cap( 2 );
					QScriptValue generator = myEngine.globalObject().property( REGEXP.cap( 1 ) );
					if ( !generator.isValid() ) {
						contents.replace( ".", " ." );
						myEngine.evaluate( contents );
						generator = myEngine.globalObject().property( REGEXP.cap( 1 ) );
					}
					//qDebug() << generator.toString();
					if ( generator.isValid() ) {
						generator.call();
						//result = myEngine.globalObject().property( DIRECTORY ).toString();
						//result = myEngine.globalObject().property( REGEXP.cap( 2 ) ).toString();
						//QRegExp variable( "(\\w+)\\W+\\w+\\W+\\}" );
						QRegExp variable( "(\\w+)[^\\n=]*=[^\\n]*\\n[^\\n]*\\}" );
						//variable.setMinimal( true );
						if ( variable.indexIn( generator.toString(), 0 ) != -1 ) {
							//qDebug() << variable.cap( 1 );
							result = myEngine.globalObject().property( variable.cap( 1 ) ).toString() + tail;
							qDebug() << result;
						}
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
#if 1
QString Utility::gnash( QString& error ) {
	QString result;
	QProcess process;
	process.start( GNASH );
	bool started = process.waitForStarted();
#ifdef Q_WS_WIN
	if ( !started ) {
		process.start( "sdl-" + GNASH );
		started = process.waitForStarted();
	}
#endif
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
#else
QString Utility::gnash() {
	QString result;
	QProcess process;
	process.start( GNASH );
	if ( process.waitForStarted() && process.waitForFinished() ) {
		QByteArray bytes = process.readAllStandardOutput();
		QString output = QString::fromUtf8( bytes.data() );
		QRegExp regexp( LISTDATAFLV );
		if ( regexp.indexIn( output ) != -1 )
			result = regexp.cap(1);
	}
	return result;
}
#endif

// ウィキからスクランブル文字列を取得する
QString Utility::wiki() {
	QDate today = QDate::currentDate();
	//QDate today = QDate( 2011, 06, 20 );
	int offset = 1 - today.dayOfWeek();		//直前の月曜までのオフセット。月曜日なら0
	if ( offset == 0 && QTime::currentTime().hour() <= 9 )	//月曜日で10時より前なら1週間前の月曜日に
		offset = -7;
	QDate monday = today.addDays( offset );

	QString result;
	QStringList attributeList;
	QXmlQuery query;
	//qDebug() << monday.toString( "yyyyMMdd" );
	query.setQuery( WIKIXML2 + monday.toString( "yyyyMMdd" ) + WIKIXML3 );
	if ( query.isValid() ) {
		query.evaluateTo( &attributeList );
		if ( attributeList.count() > 0 )
			result = attributeList[0];
	}
	return result;
}
