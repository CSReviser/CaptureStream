#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include <QThread>

#include "mainwindow.h"

class DownloadThread : public QThread {
	Q_OBJECT

public:
	DownloadThread( Ui::MainWindowClass* ui );
	~DownloadThread() {}
	void cancel() { isCanceled = true; }

protected:
	void run();

signals:
	void critical( QString message );
	void information( QString message );
	void current( QString );
	void messageWithoutBreak( QString );

private:
	QStringList getAttribute( QString url, QString attribute );
	bool checkFlvstreamer( QString& path );
	bool checkOutputDir( QString dirPath );
	void id3tag( QString fullPath, QString album, QString title, QString year, QString artist );
	bool flv2mp3( const QString& flvPath, const QString& mp3Path );
	void downloadCharo();
	void downloadENews( bool re_read );
	bool captureStream( QString kouza, QString hdate, QString file, int retryCount, bool guess = false );
	bool captureStreamPast( QString kouza, QString file, int retryCount, bool guess );
	void downloadPast( int count, QString file, QString kouza );
	void downloadOneWeek( int i, int addDays, QStringList& fileList, QStringList& kouzaList, QStringList& hdateList );
	QString formatName( QString format, QString kouza, QString hdate, QString file, bool checkIllegal );
	QStringList getElements( QString url, QString path );
	void downloadShower();

	Ui::MainWindowClass* ui;
	bool isCanceled;

	static QString paths[];
	static QString prefix;
	static QString suffix;

	static QString flv_host;
	static QString flv_app;
	static QString flv_service_prefix;

	static QString flvstreamer;
	static QString scramble;

};

#endif // DOWNLOADTHREAD_H
