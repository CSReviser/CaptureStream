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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "downloadthread.h"
#include "customizedialog.h"
#include "scrambledialog.h"
#include "utility.h"
#include "qt4qt5.h"

#ifdef QT5
#include <QXmlQuery>
#include <QDesktopWidget>
#include <QRegExp>
#endif
#ifdef QT6
#include <QRegularExpression>
#endif
#include <QMessageBox>
#include <QByteArray>
#include <QStringList>
#include <QProcess>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <QSettings>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QtNetwork>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QVariant>
#include <QFont>
#include <QDesktopServices>

#define SETTING_GROUP "MainWindow"
#define SETTING_GEOMETRY "geometry"
#define SETTING_WINDOWSTATE "windowState"
#define SETTING_MAINWINDOW_POSITION "Mainwindow_Position"
#define SETTING_SAVE_FOLDER "save_folder"
#define SETTING_SCRAMBLE "scramble"
#define SETTING_SCRAMBLE_URL1 "scramble_url1"
#define SETTING_SCRAMBLE_URL2 "scramble_url2"
#define SCRAMBLE_URL1 "http://www47.atwiki.jp/jakago/pub/scramble.xml"
#define SCRAMBLE_URL2 "http://cdn47.atwikiimg.com/jakago/pub/scramble.xml"
#define X11_WINDOW_VERTICAL_INCREMENT 5
#define KOZA_SEPARATION_FLAG false

#define SETTING_OPTIONAL1 "optional1"
#define SETTING_OPTIONAL2 "optional2"
#define SETTING_OPTIONAL3 "optional3"
#define SETTING_OPTIONAL4 "optional4"
#define SETTING_OPT_TITLE1 "opt_title1"
#define SETTING_OPT_TITLE2 "opt_title2"
#define SETTING_OPT_TITLE3 "opt_title3"
#define SETTING_OPT_TITLE4 "opt_title4"
#define OPTIONAL1 "77RQWQX1L6_01"	// ニュースで学ぶ「現代英語」
#define OPTIONAL2 "WKMNWGMN6R_01"	// アラビア語講座
#define OPTIONAL3 "GLZQ4M519X_01"	// Asian View
#define OPTIONAL4 "N13V9K157Y_01"	// ポルトガル語講座
#define Program_TITLE1 "ニュースで学ぶ「現代英語」"
#define Program_TITLE2 "アラビア語講座"
#define Program_TITLE3 "Asian View"
#define Program_TITLE4 "ポルトガル語講座"

#ifdef QT4_QT5_WIN
#define STYLE_SHEET "stylesheet-win.qss"
#else
#ifdef QT4_QT5_MAC
#define STYLE_SHEET "stylesheet-mac.qss"
#else
#define STYLE_SHEET "stylesheet-ubu.qss"
#endif
#endif

namespace {
	bool outputDirSpecified = false;
	QString version() {
		QString result;
		// 日本語ロケールではQDate::fromStringで曜日なしは動作しないのでQRegExpを使う
		// __DATE__の形式： "Jul  8 2011"
#ifdef QT5
		static QRegExp regexp( "([a-zA-Z]{3})\\s+(\\d{1,2})\\s+(\\d{4})" );
		static QStringList months = QStringList()
				<< "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun"
				<< "Jul" << "Aug" << "Sep" << "Oct" << "Nov" << "Dec";
		if ( regexp.indexIn( __DATE__ ) != -1 ) {
//			int month = months.indexOf( regexp.cap( 1 ) ) + 1;
//			int day = regexp.cap( 2 ).toInt();
//			result = QString( " (%1/%2/%3)" ).arg( regexp.cap( 3 ) )
//					.arg( month, 2, 10, QLatin1Char( '0' ) ).arg( day, 2, 10, QLatin1Char( '0' ) );
			result = QString( "  (2024/08/18)" ); 
		}
#endif
#ifdef QT6
			result = QString( "  (2024/08/18)" ); 
#endif
		return result;
	}
}

QString MainWindow::outputDir;
QString MainWindow::ini_file_path;
QString MainWindow::scramble;
QString MainWindow::scrambleUrl1;
QString MainWindow::scrambleUrl2;
QString MainWindow::optional1;
QString MainWindow::optional2;
QString MainWindow::optional3;
QString MainWindow::optional4;
QString MainWindow::program_title1;
QString MainWindow::program_title2;
QString MainWindow::program_title3;
QString MainWindow::program_title4;
QString MainWindow::prefix = "http://cgi2.nhk.or.jp/gogaku/st/xml/";
QString MainWindow::suffix = "listdataflv.xml";
QString MainWindow::json_prefix = "https://www.nhk.or.jp/radioondemand/json/";
QString MainWindow::no_write_ini;
bool MainWindow::koza_separation_flag = false;
bool MainWindow::id_flag = false;
QStringList MainWindow::idList;
QStringList MainWindow::titleList;

MainWindow::MainWindow( QWidget *parent )
		: QMainWindow( parent ), ui( new Ui::MainWindowClass ), downloadThread( NULL ) {
#ifdef QT4_QT5_MAC
	ini_file_path = Utility::ConfigLocationPath();
#endif
#if !defined( QT4_QT5_MAC )
	ini_file_path = Utility::applicationBundlePath();
#endif	
	ui->setupUi( this );
	settings( ReadMode );
	this->setWindowTitle( this->windowTitle() + version() );
	no_write_ini = "yes";
	
#ifdef QT4_QT5_MAC		// Macのウィンドウにはメニューが出ないので縦方向に縮める
//	setMaximumHeight( maximumHeight() - menuBar()->height() );
//	setMinimumHeight( maximumHeight() - menuBar()->height() );
	setMaximumHeight( maximumHeight() + ( menuBar()->height() - 24 ) * 2 );	// レコーディングボタンが表示されない問題対策　2024/06/06
	setMinimumHeight( maximumHeight() + ( menuBar()->height() - 24 ) * 2 );	// レコーディングボタンが表示されない問題対策　2024/06/06
//	QRect rect = geometry();
//	rect.setHeight( rect.height() - menuBar()->height() );
//	rect.setHeight( rect.height() );		// ダウンロードボタンが表示されない問題対策　2022/04/16
//	rect.moveTop( rect.top() + menuBar()->height() );	// 4.6.3だとこれがないとウィンドウタイトルがメニューバーに隠れる
//	setGeometry( rect );
#endif
#ifdef Q_OS_LINUX		// Linuxでは高さが足りなくなるので縦方向に伸ばしておく
	menuBar()->setNativeMenuBar(false);	// メニューバーが表示されなくなったに対応
	setMaximumHeight( maximumHeight() + X11_WINDOW_VERTICAL_INCREMENT );
	setMinimumHeight( maximumHeight() + X11_WINDOW_VERTICAL_INCREMENT );
	QRect rect = geometry();
	rect.setHeight( rect.height() + X11_WINDOW_VERTICAL_INCREMENT );
	setGeometry( rect );
#endif

//#if !defined( QT4_QT5_MAC ) && !defined( QT4_QT5_WIN )
	QPoint bottomLeft = geometry().bottomLeft();
	bottomLeft += QPoint( 0, menuBar()->height() + statusBar()->height() + 3 );
	messagewindow.move( bottomLeft );
//#endif

	// 「カスタマイズ」メニューの構築
	customizeMenu = menuBar()->addMenu( QString::fromUtf8( "カスタマイズ" ) );

	QAction* action = new QAction( QString::fromUtf8( "保存フォルダ設定..." ), this );
	connect( action, SIGNAL( triggered() ), this, SLOT( customizeSaveFolder() ) );
	customizeMenu->addAction( action );
	action = new QAction( QString::fromUtf8( "保存フォルダ開く..." ), this );
	connect( action, SIGNAL( triggered() ), this, SLOT( customizeFolderOpen() ) );
	customizeMenu->addAction( action );
	customizeMenu->addSeparator();
	action = new QAction( QString::fromUtf8( "ファイル名設定..." ), this );
	connect( action, SIGNAL( triggered() ), this, SLOT( customizeFileName() ) );
	customizeMenu->addAction( action );

	action = new QAction( QString::fromUtf8( "タイトルタグ設定..." ), this );
	connect( action, SIGNAL( triggered() ), this, SLOT( customizeTitle() ) );
	customizeMenu->addAction( action );
	customizeMenu->addSeparator();
	action = new QAction( QString::fromUtf8( "任意番組設定..." ), this );
	connect( action, SIGNAL( triggered() ), this, SLOT( customizeScramble() ) );
	customizeMenu->addAction( action );

	customizeMenu->addSeparator();
	action = new QAction( QString::fromUtf8( "設定削除（終了）..." ), this );
	connect( action, SIGNAL( triggered() ), this, SLOT( closeEvent2() ) );
	customizeMenu->addAction( action );
		
	//action = new QAction( QString::fromUtf8( "スクランブル文字列..." ), this );
	//connect( action, SIGNAL( triggered() ), this, SLOT( customizeScramble() ) );
	//customizeMenu->addAction( action );

	QString styleSheet;
	QFile real( Utility::applicationBundlePath() + STYLE_SHEET );
	if ( real.exists() ) {
		real.open( QFile::ReadOnly );
		styleSheet = QLatin1String( real.readAll() );
	} else {
		QFile res( QString( ":/" ) + STYLE_SHEET );
		res.open( QFile::ReadOnly );
		styleSheet = QLatin1String( res.readAll() );
	}
#ifdef QT4_QT5_MAC    // MacのみoutputDirフォルダに置かれたSTYLE_SHEETを優先する
	QFile real2( MainWindow::outputDir + STYLE_SHEET );
	if ( real2.exists() ) {
		real2.open( QFile::ReadOnly );
		styleSheet = QLatin1String( real2.readAll() );
	} else {
		QFile real3( Utility::appConfigLocationPath() + STYLE_SHEET );
		if ( real3.exists() ) {
			real3.open( QFile::ReadOnly );
			styleSheet = QLatin1String( real3.readAll() );
		} else {
			QFile real4( Utility::ConfigLocationPath() + STYLE_SHEET );
			if ( real4.exists() ) {
				real4.open( QFile::ReadOnly );
				styleSheet = QLatin1String( real4.readAll() );
			}
		}
	} 
#endif	
	qApp->setStyleSheet( styleSheet );

//	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // DPI support
//	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps); //HiDPI pixmaps
//	adjustSize();                             //高DPIディスプレイ対応
//	setFixedSize(size());
//        int dpiX = qApp->desktop()->logicalDpiX();
//	QFont f;
//	int defaultFontSize = f.pointSize() * ( 96.0 / dpiX );
//	f.setPointSize( defaultFontSize );
//	qApp->setFont(f);
}

MainWindow::~MainWindow() {
	if ( downloadThread ) {
		downloadThread->terminate();
		delete downloadThread;
	}
	bool nogui_flag = Utility::nogui();
	if ( !nogui_flag && no_write_ini == "yes" )
		settings( WriteMode );
	delete ui;
}

void MainWindow::closeEvent( QCloseEvent *event ) {
	Q_UNUSED( event )
	if ( downloadThread ) {
		messagewindow.appendParagraph( QString::fromUtf8( "ダウンロードをキャンセル中..." ) );
		download();
	}
	messagewindow.close();
	QCoreApplication::exit();
}

void MainWindow::settings( enum ReadWriteMode mode ) {
	typedef struct CheckBox {
		QAbstractButton* checkBox;
		QString key;
		QVariant defaultValue;
		QString titleKey;
		QVariant titleFormat;
		QString fileNameKey;
		QVariant fileNameFormat;
	} CheckBox;
#define DefaultTitle "%k_%Y_%M_%D"
#define DefaultFileName "%k_%Y_%M_%D.m4a"
	CheckBox checkBoxes[] = {
		{ ui->toolButton_basic0, "basic0", false, "basic0_title", DefaultTitle, "basic0_file_name", DefaultFileName },
		{ ui->toolButton_basic1, "basic1", false, "basic1_title", DefaultTitle, "basic1_file_name", DefaultFileName },
		{ ui->toolButton_basic2, "basic2", false, "basic2_title", DefaultTitle, "basic2_file_name", DefaultFileName },
		{ ui->toolButton_basic3, "basic3", false, "basic3_title", DefaultTitle, "basic3_file_name", DefaultFileName },
		{ ui->toolButton_timetrial, "timetrial", false, "timetrial_title", DefaultTitle, "timetrial_file_name", DefaultFileName },
		{ ui->toolButton_kaiwa, "kaiwa", false, "kaiwa_title", DefaultTitle, "kaiwa_file_name", DefaultFileName },
		{ ui->toolButton_business1, "business1", false, "business1_title", DefaultTitle, "business1_file_name", DefaultFileName },
//		{ ui->toolButton_business2, "business2", false, "business2_title", DefaultTitle, "business2_file_name", DefaultFileName },
//		{ ui->toolButton_gakusyu, "gakusyu", false, "gakusyu_title", DefaultTitle, "gakusyu_file_name", DefaultFileName },
//		{ ui->toolButton_gendai, "gendai", false, "gendai_title", DefaultTitle, "gendai_file_name", DefaultFileName },
		{ ui->toolButton_chinese, "chinese", false, "chinese_title", DefaultTitle, "chinese_file_name", DefaultFileName },
		{ ui->toolButton_french, "french", false, "french_title", DefaultTitle, "french_file_name", DefaultFileName },
		{ ui->toolButton_italian, "italian", false, "italian_title", DefaultTitle, "italian_file_name", DefaultFileName },
		{ ui->toolButton_hangeul, "hangeul", false, "hangeul_title", DefaultTitle, "hangeul_file_name", DefaultFileName },
		{ ui->toolButton_german, "german", false, "german_title", DefaultTitle, "german_file_name", DefaultFileName },
		{ ui->toolButton_spanish, "spanish", false, "spanish_title", DefaultTitle, "spanish_file_name", DefaultFileName },
//		{ ui->toolButton_levelup_chinese, "levelup-chinese", false, "levelup-chinese_title", DefaultTitle, "levelup-chinese_file_name", DefaultFileName },
//		{ ui->toolButton_levelup_hangeul, "levelup-hangeul", false, "levelup-hangeul_title", DefaultTitle, "levelup-hangeul_file_name", DefaultFileName },
		{ ui->toolButton_enjoy, "enjoy", false, "enjoy_title", DefaultTitle, "enjoy_file_name", DefaultFileName },
		{ ui->toolButton_russian, "russian", false, "russian_title", DefaultTitle, "russian_file_name", DefaultFileName },
//		{ ui->toolButton_vrradio, "vrradio", false, "vrradio_title", DefaultTitle, "vrradio_file_name", DefaultFileName },
		{ ui->toolButton_vrradio1, "vrradio1", false, "vrradio_title", DefaultTitle, "vrradio_file_name", DefaultFileName },
		{ ui->toolButton_optional1, "optional_1", false, "optional1_title", DefaultTitle, "optional1_file_name", DefaultFileName },
		{ ui->toolButton_optional2, "optional_2", false, "optional2_title", DefaultTitle, "optional2_file_name", DefaultFileName },
		{ ui->toolButton_optional3, "optional_3", false, "optional3_title", DefaultTitle, "optional3_file_name", DefaultFileName },
		{ ui->toolButton_optional4, "optional_4", false, "optional4_title", DefaultTitle, "optional4_file_name", DefaultFileName },
		{ ui->toolButton_stepup_chinese, "stepup-chinese", false, "stepup-chinese_title", DefaultTitle, "stepup-chinese_file_name", DefaultFileName },
//		{ ui->toolButton_omotenashi_chinese, "omotenashi-chinese", false, "omotenashi-chinese_title", DefaultTitle, "omotenashi-chinese_file_name", DefaultFileName },
//		{ ui->toolButton_omotenashi_hangeul, "omotenashi-hangeul", false, "omotenashi-hangeul_title", DefaultTitle, "omotenashi-hangeul_file_name", DefaultFileName },
		{ ui->toolButton_stepup_hangeul, "stepup-hangeul", false, "stepup-hangeul_title", DefaultTitle, "stepup-hangeul_file_name", DefaultFileName },
		{ ui->checkBox_13, "charo", false, "charo_title", DefaultTitle, "charo_file_name", DefaultFileName },
		{ ui->checkBox_14, "e-news", false, "e-news_title", DefaultTitle, "e-news_file_name", DefaultFileName },
		{ ui->checkBox_shower, "shower", false, "shower_title", DefaultTitle, "shower_file_name", DefaultFileName },
		{ ui->checkBox_15, "e-news-reread", false, "e-news-reread_title", DefaultTitle, "e-news-reread_file_name", DefaultFileName },
		{ ui->toolButton_skip, "skip", true, "", "", "", "" },
		{ ui->checkBox_keep_on_error, "keep_on_error", false, "", "", "", "" },
		{ ui->checkBox_this_week, "this_week", true, "", "", "", "" },
		{ ui->checkBox_next_week, "next_week", false, "", "", "", "" },
		{ ui->checkBox_next_week2, "next_week", false, "", "", "", "" },
		{ ui->checkBox_past_week, "past_week", false, "", "", "", "" },
		{ ui->toolButton_detailed_message, "detailed_message", false, "", "", "", "" },
		{ NULL, NULL, false, "", "", "", "" }
	};
	typedef struct ComboBox {
		QComboBox* comboBox;
		QString key;
		QVariant defaultValue;
	} ComboBox;
	ComboBox comboBoxes[] = {
		{ ui->comboBox_enews, "e-news-index", ENewsSaveBoth },
		{ ui->comboBox_shower, "shower_index", ENewsSaveBoth },
		{ NULL, NULL, false }
	};
	ComboBox textComboBoxes[] = {
		{ ui->comboBox_extension, "audio_extension", "m4a" },	//デフォルト拡張子をmp3からm4aに変更
		{ NULL, NULL, false }
	};

#if !defined( QT4_QT5_MAC )
	QSettings settings( Utility::applicationBundlePath() + INI_FILE, QSettings::IniFormat );
#endif
#ifdef QT4_QT5_MAC
	QSettings settings( Utility::ConfigLocationPath() + INI_FILE, QSettings::IniFormat );
#endif
	settings.beginGroup( SETTING_GROUP );

	if ( mode == ReadMode ) {	// 設定読み込み
		QVariant saved;

#if !defined( QT4_QT5_MAC )
//#if defined( QT4_QT5_MAC ) || defined( QT4_QT5_WIN )	// X11では正しく憶えられないので位置をリストアしない(2022/11/01:Linux向けに変更）
		saved = settings.value( SETTING_GEOMETRY );
//		if ( saved.type() == QVariant::Invalid )
		if ( saved.toString() == "" )
			move( 70, 22 );
		else {
			// ウィンドウサイズはバージョン毎に変わる可能性があるのでウィンドウ位置だけリストアする
			QSize windowSize = size();
			restoreGeometry( saved.toByteArray() );
			resize( windowSize );
		}
//#endif                                              　//(2022/11/01:Linux向けに変更） 
#endif
#ifdef QT4_QT5_MAC
		saved = settings.value( SETTING_MAINWINDOW_POSITION );
//		if ( saved.type() == QVariant::Invalid ) {
		if ( saved.toString() == "" ){
			move( 70, 22 );
			QRect rect = geometry();
			rect.setHeight( rect.height() );
			rect.moveTop( rect.top() );
			setGeometry( rect );
		} else {
			QSize windowSize = size();
			move( saved.toPoint() );
			resize( windowSize );
		}
		saved = settings.value( SETTING_WINDOWSTATE );
//		if ( !(saved.type() == QVariant::Invalid) )
		if ( !(saved.toString() == "" ) )
			restoreState( saved.toByteArray() );
#endif

		saved = settings.value( SETTING_SAVE_FOLDER );
#if !defined( QT4_QT5_MAC )
//		outputDir = saved.type() == QVariant::Invalid ? Utility::applicationBundlePath() : saved.toString();
		outputDir = saved.toString() == "" ? Utility::applicationBundlePath() : saved.toString();
#endif
#ifdef QT4_QT5_MAC
//		if ( saved.type() == QVariant::Invalid ) {
		if ( saved.toString() == "" ) {
			outputDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
			MainWindow::customizeSaveFolder();
		} else
			outputDir = saved.toString();
#endif

		saved = settings.value( SETTING_SCRAMBLE );
//		scramble = saved.type() == QVariant::Invalid ? "" : saved.toString();
		scramble = saved.toString() == "" ? "" : saved.toString();

		saved = settings.value( SETTING_SCRAMBLE_URL1 );
//		scrambleUrl1 = saved.type() == QVariant::Invalid ? SCRAMBLE_URL1 : saved.toString();
		scrambleUrl1 = saved.toString() == "" ? SCRAMBLE_URL1 : saved.toString();
		saved = settings.value( SETTING_SCRAMBLE_URL2 );
//		scrambleUrl2 = saved.type() == QVariant::Invalid ? SCRAMBLE_URL2 : saved.toString();
		scrambleUrl2 = saved.toString() == "" ? SCRAMBLE_URL2 : saved.toString();	
			
		saved = settings.value( SETTING_OPTIONAL1 );
//		optional1 = saved.type() == QVariant::Invalid ? OPTIONAL1 : saved.toString();
		optional1 = saved.toString() == "" ? OPTIONAL1 : saved.toString();
		saved = settings.value( SETTING_OPTIONAL2 );
//		optional2 = saved.type() == QVariant::Invalid ? OPTIONAL2 : saved.toString();
		optional2 = saved.toString() == "" ? OPTIONAL2 : saved.toString();
		saved = settings.value( SETTING_OPTIONAL3 );
//		optional3 = saved.type() == QVariant::Invalid ? OPTIONAL3 : saved.toString();
		optional3 = saved.toString() == "" ? OPTIONAL3 : saved.toString();
		saved = settings.value( SETTING_OPTIONAL4 );
//		optional4 = saved.type() == QVariant::Invalid ? OPTIONAL4 : saved.toString();
		optional4 = saved.toString() == "" ? OPTIONAL4 : saved.toString();		

		saved = settings.value( SETTING_OPT_TITLE1 );
//		program_title1 = saved.type() == QVariant::Invalid ? QString::fromUtf8( Program_TITLE1 ) : saved.toString();
		program_title1 = saved.toString() == "" ? QString::fromUtf8( Program_TITLE1 ) : saved.toString();
		saved = settings.value( SETTING_OPT_TITLE2 );
//		program_title2 = saved.type() == QVariant::Invalid ? QString::fromUtf8( Program_TITLE2 ) : saved.toString();
		program_title2 = saved.toString() == "" ? QString::fromUtf8( Program_TITLE2 ) : saved.toString();
		saved = settings.value( SETTING_OPT_TITLE3 );
//		program_title3 = saved.type() == QVariant::Invalid ? QString::fromUtf8( Program_TITLE3 ) : saved.toString();
		program_title3 = saved.toString() == "" ? QString::fromUtf8( Program_TITLE3 ) : saved.toString();
		saved = settings.value( SETTING_OPT_TITLE4 );
//		program_title4 = saved.type() == QVariant::Invalid ? QString::fromUtf8( Program_TITLE4 ) : saved.toString();
		program_title4 = saved.toString() == "" ? QString::fromUtf8( Program_TITLE4 ) : saved.toString();

		ui->toolButton_optional1->setText( QString( program_title1 ) );
		ui->toolButton_optional2->setText( QString( program_title2 ) );
		ui->toolButton_optional3->setText( QString( program_title3 ) );
		ui->toolButton_optional4->setText( QString( program_title4 ) );

//		QString opt_TITLE1 = getJsonData( optional1 );
//		QString opt_TITLE2 = getJsonData( optional2 );
//		QString opt_TITLE3 = getJsonData( optional3 );
//		QString opt_TITLE4 = getJsonData( optional4 );
		
//		program_title1 = opt_TITLE1;
//		program_title2 = opt_TITLE2;
//		program_title3 = opt_TITLE3;
//		program_title4 = opt_TITLE4;
//		
		ui->toolButton_optional1->setText( QString( program_title1 ) );
		ui->toolButton_optional2->setText( QString( program_title2 ) );
		ui->toolButton_optional3->setText( QString( program_title3 ) );
		ui->toolButton_optional4->setText( QString( program_title4 ) );

		for ( int i = 0; checkBoxes[i].checkBox != NULL; i++ ) {
			checkBoxes[i].checkBox->setChecked( settings.value( checkBoxes[i].key, checkBoxes[i].defaultValue ).toBool() );
		}
		for ( int i = 0; comboBoxes[i].comboBox != NULL; i++ )
			comboBoxes[i].comboBox->setCurrentIndex( settings.value( comboBoxes[i].key, comboBoxes[i].defaultValue ).toInt() );
		for ( int i = 0; textComboBoxes[i].comboBox != NULL; i++ ) {
			QString extension = settings.value( textComboBoxes[i].key, textComboBoxes[i].defaultValue ).toString();
			textComboBoxes[i].comboBox->setCurrentIndex( textComboBoxes[i].comboBox->findText( extension ) );
		}
		std::tie( idList, titleList ) = Utility::getProgram_List();	
	} else {	// 設定書き出し
//#if defined( QT4_QT5_MAC ) || defined( QT4_QT5_WIN )　//(2022/11/01:Linux向けに変更）
		settings.setValue( SETTING_GEOMETRY, saveGeometry() );
//#endif                                              　//(2022/11/01:Linux向けに変更）   
		if ( outputDirSpecified )
			settings.setValue( SETTING_SAVE_FOLDER, outputDir );
		settings.setValue( SETTING_SCRAMBLE, scramble );
		settings.setValue( SETTING_SCRAMBLE_URL1, scrambleUrl1 );
		settings.setValue( SETTING_SCRAMBLE_URL2, scrambleUrl2 );
		
		settings.setValue( SETTING_OPTIONAL1, optional1 );
		settings.setValue( SETTING_OPTIONAL2, optional2 );
		settings.setValue( SETTING_OPTIONAL3, optional3 );
		settings.setValue( SETTING_OPTIONAL4, optional4 );
		settings.setValue( SETTING_OPT_TITLE1, program_title1 );
		settings.setValue( SETTING_OPT_TITLE2, program_title2 );
		settings.setValue( SETTING_OPT_TITLE3, program_title3 );
		settings.setValue( SETTING_OPT_TITLE4, program_title4 );
		
		for ( int i = 0; checkBoxes[i].checkBox != NULL; i++ ) {
			settings.setValue( checkBoxes[i].key, checkBoxes[i].checkBox->isChecked() );
		}
		for ( int i = 0; comboBoxes[i].comboBox != NULL; i++ )
			settings.setValue( comboBoxes[i].key, comboBoxes[i].comboBox->currentIndex() );
		for ( int i = 0; textComboBoxes[i].comboBox != NULL; i++ )
			settings.setValue( textComboBoxes[i].key, textComboBoxes[i].comboBox->currentText() );
	}

	settings.endGroup();
}

void MainWindow::customizeTitle() {
	CustomizeDialog dialog( Ui::TitleMode );
	dialog.exec();
}

void MainWindow::customizeFileName() {
	CustomizeDialog dialog( Ui::FileNameMode );
	dialog.exec();
}

void MainWindow::customizeSaveFolder() {
	QString dir = QFileDialog::getExistingDirectory( 0, QString::fromUtf8( "書き込み可能な保存フォルダを指定してください" ),
									   outputDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
	if ( dir.length() ) {
		outputDir = dir + QDir::separator();
		outputDirSpecified = true;
	}
}

void MainWindow::customizeFolderOpen() {
	QDesktopServices::openUrl(QUrl("file:///" + outputDir + QDir::separator(), QUrl::TolerantMode));
}

void MainWindow::customizeScramble() {
	QString optional_temp[] = { optional1, optional2, optional3, optional4, "NULL" };
	ScrambleDialog dialog( optional1, optional2, optional3, optional4 );
    if (dialog.exec() ) {
    	QString pattern( "[A-Z0-9]{4} || [A-Z0-9]{10}" );
    	pattern = QRegularExpression::anchoredPattern(pattern);
	for ( int i = 0; optional_temp[i] != "NULL"; i++ ) 
	    	if ( QRegularExpression(pattern).match( optional_temp[i] ).hasMatch() ) optional_temp[i] += "_01";

	QString optional[] = { dialog.scramble1(), dialog.scramble2(), dialog.scramble3(), dialog.scramble4(), "NULL" };	
	QString title[8];
//	QStringList idList;
//	QStringList titleList;
//	std::tie( idList, titleList ) = Utility::getProgram_List();
	for ( int i = 0; optional[i] != "NULL"; i++ ) {
		optional[i] = Utility::four_to_ten( optional[i] );
		if ( idList.contains( optional[i] ) ) title[i] = titleList[idList.indexOf( optional[i] )]; 
//		for ( int k = 0; k < idList.count() ; k++ ) { if ( optional[i] == idList[k] ) {title[i] = titleList[k]; break;} }
		if ( title[i]  == "" ) { title[i] = Utility::getProgram_name( optional[i] ); }
		if ( title[i]  == "" ) { optional[i] = optional_temp[i]; title[i] = Utility::getProgram_name( optional[i] ); }
	}
	optional1 = optional[0]; optional2 = optional[1];
	optional3 = optional[2]; optional4 = optional[3];
	program_title1 = title[0]; program_title2 = title[1];
	program_title3 = title[2]; program_title4 = title[3];

	QString program_title[] = { program_title1, program_title2, program_title3, program_title4, "NULL" };
	QAbstractButton* checkboxx[] = { ui->toolButton_optional1, ui->toolButton_optional2,
					 ui->toolButton_optional3, ui->toolButton_optional4,
					 NULL
		 	};
	bool flag = false;
	for ( int i = 0; program_title[i] != "NULL"; i++ ) {
		if ( optional[i] == optional_temp[i] && checkboxx[i]->isChecked() ) flag = true; else flag = false;
				checkboxx[i]->setChecked(false);
				checkboxx[i]->setText( QString( program_title[i] ) );
				if ( flag ) checkboxx[i]->setChecked( true );
	}
	optional1 = optional[0]; optional2 = optional[1]; optional3 = optional[2]; optional4 = optional[3];
	ScrambleDialog dialog( optional1, optional2, optional3, optional4 );
    }
}

void MainWindow::download() {	//「ダウンロード」または「キャンセル」ボタンが押されると呼び出される
	if ( !downloadThread ) {	//ダウンロード実行
		if ( messagewindow.text().length() > 0 )
			messagewindow.appendParagraph( "\n----------------------------------------" );
		ui->downloadButton->setEnabled( false );
		downloadThread = new DownloadThread( ui );
		connect( downloadThread, SIGNAL( finished() ), this, SLOT( finished() ) );
		connect( downloadThread, SIGNAL( critical( QString ) ), &messagewindow, SLOT( appendParagraph( QString ) ), Qt::BlockingQueuedConnection );
		connect( downloadThread, SIGNAL( information( QString ) ), &messagewindow, SLOT( appendParagraph( QString ) ), Qt::BlockingQueuedConnection );
		connect( downloadThread, SIGNAL( current( QString ) ), &messagewindow, SLOT( appendParagraph( QString ) ) );
		connect( downloadThread, SIGNAL( messageWithoutBreak( QString ) ), &messagewindow, SLOT( append( QString ) ) );
		downloadThread->start();
		ui->downloadButton->setText( QString::fromUtf8( "キャンセル" ) );
		ui->downloadButton->setEnabled( true );
	} else {	//キャンセル
		downloadThread->disconnect();	//wait中にSIGNALが発生するとデッドロックするためすべてdisconnect
		finished();
	}
}

QString MainWindow::getJsonData( QString url ) {
	QString attribute;
	attribute.clear() ;
    	QEventLoop eventLoop;
	QNetworkAccessManager mgr;
 	QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
	const QString jsonUrl = json_prefix + url + "/bangumi_" + url + "_01.json";
	QUrl url_json( jsonUrl );
	QNetworkRequest req;
	req.setUrl(url_json);
	QNetworkReply *reply = mgr.get(req);
	eventLoop.exec(); 
	
	if (reply->error() == QNetworkReply::NoError) {
		QString strReply = (QString)reply->readAll();
		QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
		QJsonObject jsonObject = jsonResponse.object();
		QJsonObject jsonObj = jsonResponse.object();
    
		QJsonArray jsonArray = jsonObject[ "main" ].toArray();
		QJsonObject objx2 = jsonObject[ "main" ].toObject();
		attribute = objx2[ "program_name" ].toString().replace( "　", " " );
		    for (ushort i = 0xFF1A; i < 0xFF5F; ++i) {
		        attribute = attribute.replace(QChar(i), QChar(i - 0xFEE0));
		    }
		    for (ushort i = 0xFF10; i < 0xFF1A; ++i) {
		        attribute = attribute.replace( QChar(i - 0xFEE0), QChar(i) );
		    }
	}
	return attribute;
}

void MainWindow::toggled( bool checked ) {
	QObject* sender = this->sender();
	if ( sender ) {
		QToolButton* button = (QToolButton*)sender;
		QString text = button->text();
		if ( checked )
			text.insert( 0, QString::fromUtf8( "✓ " ) );
		else
			text.remove( 0, 2 );
		button->setText( text );
	}
}

void MainWindow::finished() {
	if ( downloadThread ) {
		ui->downloadButton->setEnabled( false );
		if ( downloadThread->isRunning() ) {	//キャンセルでMainWindow::downloadから呼ばれた場合
			downloadThread->cancel();
			downloadThread->wait();
			messagewindow.appendParagraph( QString::fromUtf8( "レコーディングをキャンセルしました。" ) );
		}
		delete downloadThread;
		downloadThread = NULL;
		ui->downloadButton->setText( QString::fromUtf8( "レコーディング" ) );
		ui->downloadButton->setEnabled( true );
	}
	//ui->label->setText( "" );
//	if ( Utility::nogui() )
	bool nogui_flag = Utility::nogui();
	if ( nogui_flag )
		QCoreApplication::exit();
}

void MainWindow::closeEvent2( ) {
	int res = QMessageBox::question(this, tr("設定削除"), tr("削除しますか？"));
	if (res == QMessageBox::Yes) {
	no_write_ini = "no";
	
	QFile::remove( ini_file_path + INI_FILE );
	
	if ( downloadThread ) {
		messagewindow.appendParagraph( QString::fromUtf8( "レコーディングをキャンセル中..." ) );
		download();
	}
	messagewindow.close();
	QCoreApplication::exit();
	}
}

