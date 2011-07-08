#ifndef MP3_H
#define MP3_H

#include <QByteArray>
#include <QString>
#include <QTextCodec>

namespace MP3 {
	void createTag( QByteArray& tagBytes, QString album, QString title, QString year, QString artist, QTextCodec* utf16 );
	long tagSize( QByteArray buffer );
}

#endif // MP3_H
