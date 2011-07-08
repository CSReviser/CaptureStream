#include "mp3.h"

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
}
