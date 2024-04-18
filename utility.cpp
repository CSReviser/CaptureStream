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
#include "downloadthread.h"
#include "qt4qt5.h"

#ifdef QT5
#include <QXmlQuery>
#include <QScriptEngine>
#include <QDesktopWidget>
#include <QRegExp>
#endif
#ifdef QT6
#include <QRegularExpression>
#endif
#include <QUrl>
#include <QCoreApplication>
#include <QDir>
#include <QTemporaryFile>
#include <QProcess>
#include <QFileInfo>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QDateTime>
#include <QDate>
#include <QStandardPaths>
#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QMap>

namespace {
	const QString UPUPUP( "/../../.." );
	const QString FLARE( "flare" );
	const QString GNASH( "gnash" );
	const QString GNASH_arguments { "-r0 -v http://www.nhk.or.jp/gogaku/common/swf/streaming.swf" };
	const QUrl STREAMINGSWF( "http://www.nhk.or.jp/gogaku/common/swf/streaming.swf" );
	const QString TEMPLATE( "streamingXXXXXX.swf" );

#ifdef QT5
	const QRegExp REGEXP( "function startInit\\(\\) \\{[^}]*\\}\\s*function (\\w*).*startInit\\(\\);" );
	const QRegExp PREFIX( "load\\('([A-Z0-9]*)' \\+ CONNECT_DIRECTORY" );
	const QRegExp SUFFIX( "CONNECT_DIRECTORY \\+ '(.*)/' \\+ INIT_URI" );
#endif
#ifdef QT6
	const QRegularExpression REGEXP( "function startInit\\(\\) \\{[^}]*\\}\\s*function (\\w*).*startInit\\(\\);" );
	const QRegularExpression PREFIX( "load\\('([A-Z0-9]*)' \\+ CONNECT_DIRECTORY" );
	const QRegularExpression SUFFIX( "CONNECT_DIRECTORY \\+ '(.*)/' \\+ INIT_URI" );
#endif

	const QString LISTDATAFLV( "http://www.nhk.or.jp/gogaku/common/swf/(\\w+)/listdataflv.xml" );
        const QString WIKIXML1( "doc('" );
        const QString WIKIXML2( "')/flv/scramble[@date=\"" );
	const QString WIKIXML3( "\"]/@code/string()" );
	const QString APPNAME( "CaptureStream2" );


QDate nendo_start_date = DownloadThread::nendo_start_date1;
QDate zenki_end_date = DownloadThread::zenki_end_date;
QDate kouki_start_date = DownloadThread::kouki_start_date;
QDate nendo_end_date = DownloadThread::nendo_end_date;


QMap<QString, QString> koza_zenki = { 
	{ "0953_x1", "まいにちフランス語【初級編】" },	// まいにちフランス語 初級編
	{ "0953_y1", "まいにちフランス語【応用編】" },	// まいにちフランス語 応用編
	{ "0943_x1", "まいにちドイツ語【初級編】" },	// まいにちドイツ語 初級編
	{ "0943_y1", "まいにちドイツ語【応用編】" },	// まいにちドイツ語 応用編
	{ "0948_x1", "まいにちスペイン語【初級編】" },	// まいにちスペイン語 初級編
	{ "0948_y1", "まいにちスペイン語【応用編】" },	// まいにちスペイン語 応用編
	{ "0946_x1", "まいにちイタリア語【初級編】" },	// まいにちイタリア語 初級編
	{ "0946_y1", "まいにちイタリア語【応用編】" },	// まいにちイタリア語 応用編
	{ "0956_x1", "まいにちロシア語【初級編】" },	// まいにちロシア語 初級編
	{ "0956_y1", "まいにちロシア語【応用編】" },	// まいにちロシア語 応用編
};	

QMap<QString, QString> koza_kouki = { 
	{ "0953_x1", "まいにちフランス語【入門編】" },	// まいにちフランス語 入門編
	{ "0953_y1", "まいにちフランス語【応用編】" },	// まいにちフランス語 応用編
	{ "0943_x1", "まいにちドイツ語【入門編】" },	// まいにちドイツ語 入門編
	{ "0943_y1", "まいにちドイツ語【応用編】" },	// まいにちドイツ語 応用編
	{ "0948_x1", "まいにちスペイン語【入門編】" },	// まいにちスペイン語 入門編
	{ "0948_y1", "【まいにちスペイン語応用編】" },	// まいにちスペイン語 応用編
	{ "0946_x1", "まいにちイタリア語【入門編】" },	// まいにちイタリア語 入門編
	{ "0946_y1", "まいにちイタリア語【応用編】" },	// まいにちイタリア語 応用編
	{ "0956_x1", "まいにちロシア語【入門編】" },	// まいにちロシア語 入門編
	{ "0956_y1", "まいにちロシア語【応用編】" },	// まいにちロシア語 応用編
};	

QMap<QString, QString> koza_unkown = { 
	{ "0953_x1", "まいにちフランス語【入門/初級編】" },	// まいにちフランス語 入門編
	{ "0953_y1", "まいにちフランス語【応用編】" },		// まいにちフランス語 応用編
	{ "0943_x1", "まいにちドイツ語【入門/初級編】" },	// まいにちドイツ語 入門編
	{ "0943_y1", "まいにちドイツ語【応用編】" },		// まいにちドイツ語 応用編
	{ "0948_x1", "まいにちスペイン語【入門/初級編】" },	// まいにちスペイン語 入門編
	{ "0948_y1", "まいにちスペイン語【中級/応用編】" },	// まいにちスペイン語 応用編
	{ "0946_x1", "まいにちイタリア語【入門/初級編】" },	// まいにちイタリア語 入門編
	{ "0946_y1", "まいにちイタリア語【応用編】" },		// まいにちイタリア語 応用編
	{ "0956_x1", "まいにちロシア語【入門/初級編】" },	// まいにちロシア語 入門編
	{ "0956_y1", "まいにちロシア語【応用編】" },		// まいにちロシア語 応用編
};	

}
// Macの場合はアプリケーションバンドル、それ以外はアプリケーションが含まれるディレクトリを返す
QString Utility::applicationBundlePath() {
	QString result = QCoreApplication::applicationDirPath();
//#ifdef QT4_QT5_MAC				//Macのffmpegパス不正対策　2022/04/13
//	result = QDir::cleanPath( result + UPUPUP );
//#endif
	result += QDir::separator();
	return result;
}

QString Utility::appLocaldataLocationPath() {
	QString result = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/";
	result += QDir::separator();
	return result;
}

QString Utility::appConfigLocationPath() {
	QString result = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/";
	result += QDir::separator();
	return result;
}

QString Utility::ConfigLocationPath() {
	QString result = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/";
	result += QDir::separator();
	return result;
}

QString Utility::DownloadLocationPath() {
	QString result = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/";
	result += QDir::separator();
	return result;
}

QString Utility::HomeLocationPath() {
	QString result = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/" + APPNAME + "/";
	result += QDir::separator();
	return result;
}

std::tuple<QStringList, QStringList> Utility::getProgram_List( ) {
	QStringList idList; 		idList.clear() ;
	QStringList titleList; 		titleList.clear() ;
		
	const QString jsonUrl1 = "https://www.nhk.or.jp/radio-api/app/v1/web/ondemand/corners/new_arrivals";
	const QString jsonUrl2 = "https://www.nhk.or.jp/radioondemand/json/index_v3/index.json";

	QString strReply;
	int flag = 0;
	int TimerMin = 100;
	int TimerMax = 5000;
	int Timer = TimerMin;
	int retry = 20;
	for ( int i = 0 ; i < retry ; i++ ) {
		strReply = Utility::getJsonFile( jsonUrl1, Timer );
		if ( strReply != "error" )  {
			flag = 1; break;
		}
		strReply = Utility::getJsonFile( jsonUrl2, Timer );
		if ( strReply != "error" )  {
			flag = 2; break;
		}
		if ( Timer < 500 ) Timer += 50;
		if ( Timer > 500 && Timer < TimerMax ) Timer += 100;
	}
	
	switch ( flag ) {
	case 0: idList += "error"; titleList += "error"; break;
	case 1: std::tie( idList, titleList ) = Utility::getProgram_List1( strReply ); break;
	case 2: std::tie( idList, titleList ) = Utility::getProgram_List2( strReply ); break;
	default: idList += "error"; titleList += "error"; break;
	}
	return { idList, titleList };
}


std::tuple<QStringList, QStringList> Utility::getProgram_List1( QString strReply ) {
	QStringList attribute1; 	attribute1.clear() ;
	QStringList attribute2; 	attribute2.clear() ;
	
	QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
	QJsonObject jsonObject = jsonResponse.object();
    
	QJsonArray jsonArray = jsonObject[ "corners" ].toArray();
	for (const auto&& value : jsonArray) {
		QJsonObject objxx = value.toObject();
		QString title = objxx[ "title" ].toString();
		QString corner_name = objxx[ "corner_name" ].toString();
		QString series_site_id = objxx[ "series_site_id" ].toString();
		QString corner_site = objxx[ "corner_site_id" ].toString();
		
		QString program_name = Utility::getProgram_name3( title, corner_name );
			
		QString url_id = series_site_id + "_" + corner_site;
			
		attribute1 += url_id;
		attribute2 += program_name;
	}
	return { attribute1, attribute2 };
}

std::tuple<QStringList, QStringList> Utility::getProgram_List2( QString strReply ) {
	QStringList attribute1; 	attribute1.clear() ;
	QStringList attribute2; 	attribute2.clear() ;
	
	QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
	QJsonObject jsonObject = jsonResponse.object();
    
	QJsonArray jsonArray = jsonObject[ "data_list" ].toArray();
	for (const auto&& value : jsonArray) {
		QJsonObject objxx = value.toObject();
		QString title = objxx[ "program_name" ].toString();

		QString corner_name = objxx[ "corner_name" ].toString();
		QString series_site_id = objxx[ "site_id" ].toString();
		QString corner_site = objxx[ "corner_id" ].toString();
		
		QString program_name = Utility::getProgram_name3( title, corner_name );

		QString url_id = series_site_id + "_" + corner_site;
			
		attribute1 += url_id;
		attribute2 += program_name;
	}
	return { attribute1, attribute2 };
}


QString Utility::getJsonFile( QString jsonUrl, int Timer ) {
    	QEventLoop eventLoop;
    	QString attribute;
	QTimer timer;    
	timer.setSingleShot(true);
	QNetworkAccessManager mgr;
	QObject::connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
 	QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*) ), &eventLoop, SLOT(quit()));
	QUrl url_json( jsonUrl );
	QNetworkRequest req;
	req.setUrl(url_json);
	timer.start(Timer);  // use miliseconds
	QNetworkReply *reply = mgr.get(req);
	eventLoop.exec(); // blocks stack until "finished()" has been called

	if(timer.isActive()) {
		timer.stop();
		
		if (reply->error() == QNetworkReply::NoError) {
			attribute = (QString)reply->readAll();
		} else {
			return "error";
		}  
	} else {
          // timeout
		QObject::disconnect(&mgr, SIGNAL(finished(QNetworkReply*) ), &eventLoop, SLOT(quit()));
		reply->abort();
		return "error";
	}
	return attribute;
}

QString Utility::getProgram_name( QString url ) {
	QString attribute;	QString title;	QString corner_name;
	attribute.clear() ;
	int json_ohyo = 0 ;
	QString url_tmp = url;
	if ( url.contains( "_x1" ) ) { url.replace( "_x1", "_01" ); json_ohyo = 1 ; };
	if ( url.contains( "_y1" ) ) { url.replace( "_y1", "_01" ); json_ohyo = 2 ; };
	if (json_ohyo != 0){
		if ( QDate::currentDate() < zenki_end_date ) 
			if ( koza_zenki.contains( url_tmp ) ) return koza_zenki.value( url_tmp );
		if ( QDate::currentDate() < nendo_end_date )
			if ( koza_kouki.contains( url_tmp ) ) return koza_kouki.value( url_tmp );
		if ( koza_unkown.contains( url_tmp ) ) return koza_unkown.value( url_tmp );
	}
	QString pattern( "[0-9]{4}" );
    	pattern = QRegularExpression::anchoredPattern(pattern);
 	QString pattern2( "[A-Z0-9][0-9]{3}_[0-9]{2}" );
    	if ( QRegularExpression(pattern).match( url ).hasMatch() ) url += "_01";
    	
    	if ( !(QRegularExpression(pattern2).match( url ).hasMatch()) ) return attribute;
	
 	const QString jsonUrl1 = "https://www.nhk.or.jp/radio-api/app/v1/web/ondemand/series?site_id=" + url.left(4) + "&corner_site_id=" + url.right(2);
	const QString jsonUrl2 = "https://www.nhk.or.jp/radioondemand/json/" + url.left(4) + "/bangumi_" + url + ".json";

	QString strReply;
	int flag = 0;
	int TimerMin = 100;
	int TimerMax = 5000;
	int Timer = TimerMin;
	int retry = 20;
	for ( int i = 0 ; i < retry ; i++ ) {
		strReply = Utility::getJsonFile( jsonUrl1, Timer );
		if ( strReply != "error" )  {
			flag = 1; break;
		}
		strReply = Utility::getJsonFile( jsonUrl2, Timer );
		if ( strReply != "error" )  {
			flag = 2; break;
		}
		if ( Timer < 500 ) Timer += 50;
		if ( Timer > 500 && Timer < TimerMax ) Timer += 100;
	}
	
	switch ( flag ) {
	case 0: return attribute;
	case 1: std::tie( title, corner_name ) = Utility::getProgram_name1( strReply ); break;
	case 2: std::tie( title, corner_name ) = Utility::getProgram_name2( strReply ); break;
	default: return attribute;
	}
	attribute = Utility::getProgram_name3( title, corner_name );
	return attribute;
}

std::tuple<QString, QString> Utility::getProgram_name1( QString strReply ) {
	QString attribute;
	attribute.clear() ;
	
	QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
	QJsonObject jsonObject = jsonResponse.object();
	attribute = jsonObject[ "title" ].toString().replace( "　", " " );
	QString corner_name = jsonObject[ "corner_name" ].toString().remove( "を聴く" ).replace( "　", " " );
	return { attribute, corner_name };
}

std::tuple<QString, QString> Utility::getProgram_name2( QString strReply ) {
	QString attribute;
	attribute.clear() ;
	
	QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
	QJsonObject jsonObject = jsonResponse.object();
	QJsonObject objx2 = jsonObject[ "main" ].toObject();
	attribute = objx2[ "program_name" ].toString().replace( "　", " " );
	QString corner_name = objx2[ "corner_name" ].toString().remove( "を聴く" ).replace( "　", " " );
	return { attribute, corner_name };
}

QString Utility::getProgram_name3( QString title, QString corner_name ) {
	QString attribute = title.replace( "　", " " );
		
	if ( !(corner_name.isNull()  || corner_name.isEmpty()) ) {
		if( corner_name.contains( "曜日放送", Qt::CaseInsensitive ) || corner_name.contains( "曜放送", Qt::CaseInsensitive ) || corner_name.contains( "特集", Qt::CaseInsensitive )){
			attribute = title + "-" + corner_name;
		} else {
			attribute = corner_name;
		}
	}
	for (ushort i = 0xFF1A; i < 0xFF5F; ++i) {
		attribute = attribute.replace(QChar(i), QChar(i - 0xFEE0));
	}
	for (ushort i = 0xFF10; i < 0xFF1A; ++i) {
		attribute = attribute.replace( QChar(i - 0xFEE0), QChar(i) );
	}

	attribute = attribute.remove( "【らじる文庫】" ).remove( "より" ).remove( "カルチャーラジオ" ).remove( "【恋する朗読】" ).remove( "【ラジオことはじめ】" ).remove( "【生朗読！】" );
        attribute.replace( QString::fromUtf8( "初級編" ), QString::fromUtf8( "【初級編】" ) ); attribute.replace( QString::fromUtf8( "入門編" ), QString::fromUtf8( "【入門編】" ) );
        attribute.replace( QString::fromUtf8( "中級編" ), QString::fromUtf8( "【中級編】" ) ); attribute.replace( QString::fromUtf8( "応用編" ), QString::fromUtf8( "【応用編】" ) );
	return attribute;
}

std::tuple<QStringList, QStringList, QStringList, QStringList, QStringList> Utility::getJsonData1( QString strReply, int json_ohyo ) {
	QStringList fileList;			fileList.clear();
	QStringList kouzaList;			kouzaList.clear();
	QStringList file_titleList;		file_titleList.clear();
	QStringList hdateList;			hdateList.clear();
	QStringList yearList;			yearList.clear();

	if ( strReply != "error" ) {
		QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
		QJsonObject jsonObject = jsonResponse.object();
 
		QString program_name = jsonObject[ "title" ].toString().replace( "　", " " );
		QString corner_name = jsonObject[ "corner_name" ].toString().replace( "　", " " );
		if ( !(corner_name.isNull()  || corner_name.isEmpty()) ) {
			corner_name.remove( "を聴く" );
			if( corner_name.contains( "曜日放送", Qt::CaseInsensitive ) || corner_name.contains( "曜放送", Qt::CaseInsensitive ) || corner_name.contains( "特集", Qt::CaseInsensitive )){
				program_name = program_name + " - " + corner_name;
			} else {
				program_name = corner_name;
			}
		}
		    for (ushort i = 0xFF1A; i < 0xFF5F; ++i) {
		        program_name = program_name.replace(QChar(i), QChar(i - 0xFEE0));
		    }
		    for (ushort i = 0xFF10; i < 0xFF1A; ++i) {
		        program_name = program_name.replace( QChar(i - 0xFEE0), QChar(i) );
		    }
    
		QJsonArray jsonArray = jsonObject[ "episodes" ].toArray();
		if ( jsonArray.isEmpty() ) { QStringList xxxx; xxxx += "\0"; kouzaList += program_name; return { xxxx, kouzaList, xxxx, xxxx, xxxx };}
		for (const auto&& value : jsonArray) {
			QJsonObject objxx = value.toObject();
			QString file_title = objxx[ "program_title" ].toString();
			QString file_name = objxx[ "stream_url" ].toString();
			QString aa_contents_id = objxx[ "aa_contents_id" ].toString();
			QString onair_date = objxx[ "onair_date" ].toString();
			QRegularExpression rx("....-..-..");
			QRegularExpressionMatch match = rx.match( aa_contents_id ); 
			QString year = match.captured(0);
			year = year.left(4);
			
			QString program_name_tmp = program_name;
			if( json_ohyo == 1 && ( file_title.contains( "中級編", Qt::CaseInsensitive) || file_title.contains( "応用編", Qt::CaseInsensitive) )  ) continue;
			if( json_ohyo == 2 && ( file_title.contains( "入門編", Qt::CaseInsensitive) || file_title.contains( "初級編", Qt::CaseInsensitive) )  ) continue;
			if( json_ohyo == 1 && ( file_title.contains( "入門編", Qt::CaseInsensitive) )) program_name_tmp = program_name_tmp + " 入門編";
			if( json_ohyo == 1 && ( file_title.contains( "初級編", Qt::CaseInsensitive) )) program_name_tmp = program_name_tmp + " 初級編";
			if( json_ohyo == 2 && ( file_title.contains( "中級編", Qt::CaseInsensitive) )) program_name_tmp = program_name_tmp + " 中級編";
			if( json_ohyo == 2 && ( file_title.contains( "応用編", Qt::CaseInsensitive) )) program_name_tmp = program_name_tmp + " 応用編";
			
			kouzaList += program_name_tmp;
			file_titleList += file_title;
			fileList += file_name;
			hdateList += onair_date;
			yearList += year;
		}
	}
	return { fileList, kouzaList, file_titleList, hdateList, yearList };
}

std::tuple<QStringList, QStringList, QStringList, QStringList, QStringList> Utility::getJsonData2( QString strReply, int json_ohyo ) {
	QStringList fileList;			fileList.clear();
	QStringList kouzaList;			kouzaList.clear();
	QStringList file_titleList;		file_titleList.clear();
	QStringList hdateList;			hdateList.clear();
	QStringList yearList;			yearList.clear();

	if ( strReply != "error" ) {
		QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
		QJsonObject jsonObject = jsonResponse.object();
    
		QJsonArray jsonArray = jsonObject[ "main" ].toArray();
		QJsonObject objx2 = jsonObject[ "main" ].toObject();
		QString program_name = objx2[ "program_name" ].toString().replace( "　", " " );
		QString corner_name = objx2[ "corner_name" ].toString().replace( "　", " " );
		if ( !(corner_name.isNull()  || corner_name.isEmpty()) ) {
			corner_name.remove( "を聴く" );
			if( corner_name.contains( "曜日放送", Qt::CaseInsensitive ) || corner_name.contains( "曜放送", Qt::CaseInsensitive ) || corner_name.contains( "特集", Qt::CaseInsensitive )){
				program_name = program_name + " - " + corner_name;
			} else {
				program_name = corner_name;
			}
		}
		    for (ushort i = 0xFF1A; i < 0xFF5F; ++i) {
		        program_name = program_name.replace(QChar(i), QChar(i - 0xFEE0));
		    }
		    for (ushort i = 0xFF10; i < 0xFF1A; ++i) {
		        program_name = program_name.replace( QChar(i - 0xFEE0), QChar(i) );
		    }
		QJsonArray detail_list = objx2[ "detail_list" ].toArray();
		for (const auto&& value : detail_list) {
			QJsonObject objxx = value.toObject();
			QJsonArray file_list = objxx[ "file_list" ].toArray();					
			for (const auto&& value : file_list) {
				QJsonObject objxx2 = value.toObject();
				QString file_title = objxx2[ "file_title" ].toString();
				QString file_name = objxx2[ "file_name" ].toString();
				QString aa_vinfo4 = objxx2[ "aa_vinfo4" ].toString();
				QString onair_date = objxx2[ "onair_date" ].toString();
				QString open_time = objxx2[ "open_time" ].toString();
				QString year = aa_vinfo4.left( 4 );
				if ( year == "" ) year = open_time.left( 4 );
				
				QString program_name_tmp = program_name;
				if( json_ohyo == 1 && ( file_title.contains( "中級編", Qt::CaseInsensitive) || file_title.contains( "応用編", Qt::CaseInsensitive) )  ) continue;
				if( json_ohyo == 2 && ( file_title.contains( "入門編", Qt::CaseInsensitive) || file_title.contains( "初級編", Qt::CaseInsensitive) )  ) continue;
				if( json_ohyo == 1 && ( file_title.contains( "入門編", Qt::CaseInsensitive) )) program_name_tmp = program_name_tmp + " 入門編";
				if( json_ohyo == 1 && ( file_title.contains( "初級編", Qt::CaseInsensitive) )) program_name_tmp = program_name_tmp + " 初級編";
				if( json_ohyo == 2 && ( file_title.contains( "中級編", Qt::CaseInsensitive) )) program_name_tmp = program_name_tmp + " 中級編";
				if( json_ohyo == 2 && ( file_title.contains( "応用編", Qt::CaseInsensitive) )) program_name_tmp = program_name_tmp + " 応用編";

				kouzaList += program_name_tmp;
				file_titleList += file_title;
				fileList += file_name;
				hdateList += onair_date;
				yearList += year;
        		}					
		}
	}
	return { fileList, kouzaList, file_titleList, hdateList, yearList };
}

bool Utility::nogui() {
	return QCoreApplication::arguments().contains( "-nogui" );
}

QStringList Utility::optionList() {
	QStringList attribute;
	QStringList ProgList = QCoreApplication::arguments();
	if ( ProgList.count() < 3 ) { attribute += "erorr" ; return attribute; }
	ProgList.removeAt(0);
	QStringList idList;
	QStringList titleList;
	std::tie( idList, titleList ) = Utility::getProgram_List();

	if( Utility::nogui() ) {
		for( int i = 0; i < ProgList.count() ; i++ ){
			if ( koza_unkown.contains( ProgList[i] ) ) { attribute += ProgList[i]; continue; }
			QString pattern( "[0-9]{4}" );
    			pattern = QRegularExpression::anchoredPattern(pattern);
			if ( QRegularExpression(pattern).match( ProgList[i] ).hasMatch() ) ProgList[i] += "_01";
			if ( idList.contains( ProgList[i] ) ) attribute += ProgList[i];
		}
		if ( attribute.count() < 1 ) attribute += "return" ;
	}
	return attribute;
}

std::tuple<QString, QString, QString, QString> Utility::nogui_option( QString titleFormat, QString fileNameFormat, QString outputDir, QString extension ) {
	QString titleFormat_out = titleFormat;
	QString fileNameFormat_out = fileNameFormat;
	QString outputDir_out = outputDir;
	QString extension_out = extension;
	QStringList optionList = QCoreApplication::arguments();
	optionList.removeAt(0);

	if ( optionList.contains( "-t" ) ) { titleFormat_out = optionList[ optionList.indexOf( "-t" ) + 1 ].remove( "'" ).remove( "\"" );}
	if ( optionList.contains( "-f" ) ) { fileNameFormat_out = optionList[ optionList.indexOf( "-f" ) + 1 ].remove( "'" ).remove( "\"" );}
	if ( optionList.contains( "-o" ) ) { outputDir_out = optionList[ optionList.indexOf( "-o" ) + 1 ].remove( "'" ).remove( "\"" ) + QDir::separator();}
	if ( optionList.contains( "-e" ) ) { extension_out = optionList[ optionList.indexOf( "-e" ) + 1 ].remove( "'" ).remove( "\"" ); if (extension_out == "mp3") extension_out += "-64k-S"; }

	return { titleFormat_out, fileNameFormat_out, outputDir_out, extension_out };
}

