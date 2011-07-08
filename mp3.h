/*
   Copyright (C) 2009-2011 jakago

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

#ifndef MP3_H
#define MP3_H

#include <QByteArray>
#include <QString>
#include <QTextCodec>

namespace MP3 {
	void createTag( QByteArray& tagBytes, QString album, QString title, QString year, QString artist, QTextCodec* utf16 );
	long tagSize( QByteArray buffer );
	bool flv2mp3( const QString& flvPath, const QString& mp3Path, QString& error );
	bool id3tag( QString fullPath, QString album, QString title, QString year, QString artist, QString error );
}

#endif // MP3_H
