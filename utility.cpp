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

#include "utility.h"
#include "urldownloader.h"
#include "mainwindow.h"
#include "qt4qt5.h"

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
	const QRegExp PREFIX( "load\\('([A-Z0-9]*)' \\+ CONNECT_DIRECTORY" );
	const QRegExp SUFFIX( "CONNECT_DIRECTORY \\+ '(.*)/' \\+ INIT_URI" );

	const QString LISTDATAFLV( "http://www.nhk.or.jp/gogaku/common/swf/(\\w+)/listdataflv.xml" );
    const QString WIKIXML1( "doc('" );
    const QString WIKIXML2( "')/flv/scramble[@date=\"" );
	const QString WIKIXML3( "\"]/@code/string()" );
}

// Macの場合はアプリケーションバンドル、それ以外はアプリケーションが含まれるディレクトリを返す
QString Utility::applicationBundlePath() {
	QString result = QCoreApplication::applicationDirPath();
//#ifdef QT4_QT5_MAC				//Macのffmpegパス不正対策　2022/04/14
//	result = QDir::cleanPath( result + UPUPUP );
//#endif
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
					QString prefix;
					if ( PREFIX.indexIn( contents, 0 ) != -1 )
						prefix = PREFIX.cap( 1 );
					QString suffix;
					if ( SUFFIX.indexIn( contents, 0 ) != -1 )
						suffix = SUFFIX.cap( 1 );
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
							result = prefix + myEngine.globalObject().property( variable.cap( 1 ) ).toString() + suffix;
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

#if 0
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
    if ( !started ) {
		process.start( applicationBundlePath() + GNASH );
		started = process.waitForStarted();
	}
    if ( !started ) {
		process.start( applicationBundlePath() + "sdl-" + GNASH );
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
#endif

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
	query.setQuery( WIKIXML1 + MainWindow::scrambleUrl1 + WIKIXML2 + monday.toString( "yyyyMMdd" ) + WIKIXML3 );
	if ( query.isValid() ) {
		query.evaluateTo( &attributeList );
		if ( attributeList.count() > 0 )
			result = attributeList[0];
	}
    // urlが転送されるようになった問題に対応
    if ( result.length() <= 0 ) {
        query.setQuery( WIKIXML1 + MainWindow::scrambleUrl2 + WIKIXML2 + monday.toString( "yyyyMMdd" ) + WIKIXML3 );
        if ( query.isValid() ) {
            query.evaluateTo( &attributeList );
            if ( attributeList.count() > 0 )
                result = attributeList[0];
        }
    }
	return result;
}

bool Utility::nogui() {
	return QCoreApplication::arguments().contains( "-nogui" );
}
