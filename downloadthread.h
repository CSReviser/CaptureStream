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

#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include <QThread>
#include <QStringList>
#include <QHash>
#include <QProcess>

#include "mainwindow.h"

class DownloadThread : public QThread {
	Q_OBJECT

public:
	DownloadThread( Ui::MainWindowClass* ui );
	~DownloadThread() {}
	void cancel() { isCanceled = true; }

	static QString opt_title1;
	static QString opt_title2;
	static QString opt_title3;
	static QString opt_title4;
	static QString nendo1;
	static QString nendo2;
	static QDate nendo_end_date1;
	static QDate nendo_end_date2;
	static QDate nendo_start_date1;
	static QDate nendo_start_date2;
	static QDate nendo_start_date;
	static QDate zenki_end_date;
	static QDate kouki_start_date;
	static QDate nendo_end_date;
	
protected:
	void run();

signals:
	void critical( QString message );
	void information( QString message );
	void current( QString );
	void messageWithoutBreak( QString );

private:
	std::tuple<QStringList, QStringList, QStringList, QStringList, QStringList> getAttribute( QString url );
	std::tuple<QStringList, QStringList, QStringList, QStringList, QStringList> getJsonData( QString url );
	std::tuple<QStringList, QStringList, QStringList, QStringList, QStringList> getJsonData1( QString url );
	std::tuple<QStringList, QStringList, QStringList, QStringList, QStringList> getJsonData2( QString url );
	QString getJsonFile( QString jsonUrl );
	bool checkExecutable( QString path );
	bool isFfmpegAvailable( QString& path );
	bool istestAvailable( QString& path );
	bool checkOutputDir( QString dirPath );
	void downloadENews( bool re_read );
	QString ffmpeg_process( QStringList arguments );
		
	bool captureStream( QString kouza, QString hdate, QString file, QString nendo, QString dir );
	bool captureStream_json( QString kouza, QString hdate, QString file, QString nendo, QString title, QString dupnmb, QString json_path, bool nogui_flag, QString this_week );

	QString formatName( QString format, QString kouza, QString hdate, QString file, QString nendo, QString dupnmb, bool checkIllegal );
	QStringList getElements( QString url, QString path );
	void downloadShower();

	Ui::MainWindowClass* ui;
	bool isCanceled;
	bool failed1935;
        bool ff_end;
	static QString Error_mes;
	
	static QString paths[];
	static QString paths2[];
	static QString paths3[];
	static QString json_paths[];
	static QMap<QString, QString> map;
	static QString prefix;
	static QString prefix1;
	static QString prefix2;
	static QString prefix3;
	static QString suffix;
	static QString suffix1;
	static QString suffix2;
	static QString suffix3;
	static QString json_prefix;

	static QString flv_host;
	static QString flv_app;
	static QString flv_service_prefix;

	static QString flvstreamer;
	static QString ffmpeg;
	static QString test;
	static QString scramble;
	static QStringList malformed;
	
	static QString optional1;
	static QString optional2;
	static QString optional3;
	static QString optional4;

	static QHash<QString, QString> ffmpegHash;
	static QHash<QProcess::ProcessError, QString>processError;

};

#endif // DOWNLOADTHREAD_H
