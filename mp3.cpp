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

#include "mp3.h"

#include <QFile>

namespace MP3 {
	struct id3v22_header {
		char identifier[3];	// "ID3"
		char version[2];	// ID3v2.2.0 -> $02 00
		char flags;			// %ab000000 -> a:Unsynchronisation, b:compression
		char size[4];		// 4 * %0xxxxxxx, excluding the header (total tag size - 10)
	};

	struct id3v22_frame_header {
		char identifier[3];
		char size[3];	// excluding frame header (frame size - 6)
	};

	void encodeSize( long int32, char* byte4 ) {
		byte4[0] = (int32 & (0x0000007f << 21)) >> 21;
		byte4[1] = (int32 & (0x0000007f << 14)) >> 14;
		byte4[2] = (int32 & (0x0000007f << 7)) >> 7;
		byte4[3] = int32 & 0x0000007f;
	}

	long decodeSize( const char* byte4 ) {
		return ((long)(byte4[0] & 0x7f) << 21) + ((long)(byte4[1] & 0x7f) << 14) +
				((long)(byte4[2] & 0x7f) << 7) + (long)(byte4[3] & 0x7f);
	}

	void asciiFrame( QByteArray& frames, id3v22_frame_header& frameHeader, QString string ) {
		static const char asciiMark = '\0';
		QByteArray asciiBytes = string.toAscii();
		long length = asciiBytes.size() + 2;	// 末尾の１バイトの'\0'と文字コードの指定に１バイト
		if ( length > 2 && length < 0x01000000 ) {
			frameHeader.size[0] = (length & 0x00ff0000) >> 16;
			frameHeader.size[1] = (length & 0x0000ff00) >> 8;
			frameHeader.size[2] = length & 0x000000ff;
			frames += QByteArray( (const char*)&frameHeader, sizeof (frameHeader) );
			frames += asciiMark;
			frames += asciiBytes;
			frames += '\0';
		}
	}

	void unicodeFrame( QByteArray& frames, id3v22_frame_header& frameHeader, QString string, QTextCodec* utf16 ) {
		static const char unicodeMark = '\1';
		QByteArray utf16Bytes = utf16->fromUnicode( string );
		long length = utf16Bytes.size() + 3;	// 末尾の２バイトの'\0'と文字コードの指定に１バイト
		if ( length > 3 && length < 0x01000000 ) {
			frameHeader.size[0] = (length & 0x00ff0000) >> 16;
			frameHeader.size[1] = (length & 0x0000ff00) >> 8;
			frameHeader.size[2] = length & 0x000000ff;
			frames += QByteArray( (const char*)&frameHeader, sizeof (frameHeader) );
			frames += unicodeMark;
			frames += utf16Bytes;
			frames += '\0';
			frames += '\0';
		}
	}

	void createTag( QByteArray& tagBytes, QString album, QString title, QString year, QString artist, QTextCodec* utf16 ) {
		if ( utf16 ) {
			id3v22_header header = { { 'I', 'D', '3' }, { '\x02', '\x00' }, '\0', {} };
			id3v22_frame_header albumHeader = { { 'T', 'A', 'L' }, {} };
			id3v22_frame_header titleHeader = { { 'T', 'T', '2' }, {} };
			id3v22_frame_header yearHeader = { { 'T', 'Y', 'E' }, {} };	//numeric string, four characters long
			id3v22_frame_header artistHeader = { { 'T', 'P', '1' }, {} };
			id3v22_frame_header genreHeader = { { 'T', 'C', 'O' }, {} };
			QByteArray frames;

			unicodeFrame( frames, albumHeader, album, utf16 );
			unicodeFrame( frames, titleHeader, title, utf16 );
			asciiFrame( frames, yearHeader, year );
			unicodeFrame( frames, artistHeader, artist, utf16 );
			asciiFrame( frames, genreHeader, "(101)" );

			if ( frames.size() ) {
				encodeSize( frames.size(), header.size );
				tagBytes = QByteArray( (const char*)&header, sizeof (header) );
				tagBytes += frames;
			}
		}
	}

	// 2.2.0~2.4.0のタグのサイズを計算する
	long tagSize( QByteArray buffer ) {
		//static const int offset_identifier = 0;
		static const int offset_version = 3;
		//static const int offset_flags = 5;
		static const int offset_size = 6;
		static const int offset_data = 10;
		static const char* identifier = "ID3";

		long result = 0;

		const char* data = buffer.constData();
		const int length = buffer.size();
		if ( length > 10 && !strncmp( data, identifier, offset_version ) ) {
			// ID3v2.2.0 と ID3v2.3.0 のみサポート
			if ( data[offset_version] >= 2 && data[offset_version] <= 4 && data[offset_version + 1] == 0 )
				result = decodeSize( data + offset_size ) + offset_data;
		}

		return result;
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

	bool flv2mp3( const QString& flvPath, const QString& mp3Path, QString& error ) {
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
			result = true;
		} catch ( QString& message ) {
			QFile::remove( mp3Path );
			error = message;
		}

		return result;
	}

}
