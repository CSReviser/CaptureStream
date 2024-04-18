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

#ifndef UTILITY_H
#define UTILITY_H

#include <QString>
#include <QDate>

namespace Utility {
	QString applicationBundlePath();
	QString appLocaldataLocationPath();
	QString appConfigLocationPath();
	QString ConfigLocationPath();
	QString DownloadLocationPath();	
	QString HomeLocationPath();
	QString flare( QString& error );
	QString gnash( QString& error );
	QString wiki();
	QString getJsonFile( QString jsonUrl, int Timer );
	QString getProgram_name( QString url );
	std::tuple<QString, QString> getProgram_name1( QString strReply );
	std::tuple<QString, QString> getProgram_name2( QString strReply );
	QString getProgram_name3( QString title, QString corner_name );
	std::tuple<QStringList, QStringList> getProgram_List();
	std::tuple<QStringList, QStringList> getProgram_List1( QString strReply );
	std::tuple<QStringList, QStringList> getProgram_List2( QString strReply );
	std::tuple<QStringList, QStringList, QStringList, QStringList, QStringList> getJsonData1( QString strReply, int json_ohyo );
	std::tuple<QStringList, QStringList, QStringList, QStringList, QStringList> getJsonData2( QString strReply, int json_ohyo );
	std::tuple<QString, QString, QString, QString> nogui_option( QString titleFormat, QString fileNameFormat, QString outputDir, QString extension );
	bool nogui();
	QStringList optionList();
}

#endif // UTILITY_H
