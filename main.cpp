#include <QtGui/QApplication>
#include "mainwindow.h"
#include <stdio.h>

bool oneTime = false;

int main(int argc, char *argv[])
{
#if defined(QT_NO_DEBUG)
#ifdef Q_WS_WIN
	const char* null = "nul";
#else
	const char* null = "/dev/null";
#endif
	freopen( null, "a", stdout );
	freopen( null, "a", stderr );
#endif

	QApplication a(argc, argv);
	// 直接argcを参照してはいけない。ダブルクリックして実行するとargcは2になっている
	oneTime = QCoreApplication::arguments().size() > 1;
	MainWindow w;
	oneTime ? w.download() : w.show();
	return a.exec();
}
