#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QMutex>
#include <QCloseEvent>
#include "messagewindow.h"

#define INI_FILE "CaptureStream.ini"

//ニュースで英会話「音声と動画」「音声のみ」「動画のみ」
#define ENewsSaveBoth	0
#define ENewsSaveAudio	1
#define ENewsSaveMovie	2

class DownloadThread;

namespace Ui {
    class MainWindowClass;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

	enum ReadWriteMode {
		ReadMode, WriteMode
	};

public:
	MainWindow( QWidget *parent = 0 );
    ~MainWindow();

	static QString outputDir;
	static QString scramble;

protected:
	virtual void closeEvent( QCloseEvent *event );

public slots:
	void download();

private slots:
	void finished();
	void customizeTitle();
	void customizeFileName();
	void customizeSaveFolder();
	void customizeScramble();

private:
    Ui::MainWindowClass *ui;
	DownloadThread* downloadThread;
	QMenu* customizeMenu;
	MessageWindow messagewindow;

	void settings( enum ReadWriteMode mode );
};

#endif // MAINWINDOW_H
