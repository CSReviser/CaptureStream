#include "customizedialog.h"
#include "mainwindow.h"
#include "utility.h"

#include <QDir>
#include <QSettings>

#define SETTING_GROUP "CustomizeDialog"
#define DefaultTitle "%k_%Y_%M_%D"
#define DefaultFileName "%k_%Y_%M_%D.mp3"

typedef struct LineEdit {
	QLineEdit* lineEdit;
	QString titleKey;
	QString fileNameKey;
} LineEdit;

QStringList CustomizeDialog::courses = QStringList()
		<< QString::fromUtf8( "基礎英語1" ) << QString::fromUtf8( "基礎英語2" )
		<< QString::fromUtf8( "基礎英語3" ) << QString::fromUtf8( "英語5分間トレーニング" )
		<< QString::fromUtf8( "ラジオ英会話" ) << QString::fromUtf8( "入門ビジネス英語" )
		<< QString::fromUtf8( "実践ビジネス英語" ) << QString::fromUtf8( "リトル・チャロ2" )
		<< QString::fromUtf8( "まいにち中国語" ) << QString::fromUtf8( "まいにちフランス語" )
		<< QString::fromUtf8( "まいにちイタリア語" ) << QString::fromUtf8( "まいにちハングル講座" )
		<< QString::fromUtf8( "まいにちドイツ語" ) << QString::fromUtf8( "まいにちスペイン語" )
		<< QString::fromUtf8( "ニュースで英会話" ) << QString::fromUtf8( "ニュースで英会話（読み直し音声）" );
QStringList CustomizeDialog::titleKeys = QStringList()
		<< "basic1_title" << "basic2_title" << "basic3_title" << "training_title"
		<< "kaiwa_title" << "business1_title" << "business2_title"  << "charo_title"
		<< "chinese_title" << "french_title" << "italian_title" << "hangeul_title"
		<< "german_title" << "spanish_title" << "e-news_title" << "e-news-reread_title";
QStringList CustomizeDialog::fileNameKeys = QStringList()
		<< "basic1_file_name" << "basic2_file_name" << "basic3_file_name" << "training_file_name"
		<< "kaiwa_file_name" << "business1_file_name" << "business2_file_name" << "charo_file_name"
		<< "chinese_file_name" << "french_file_name" << "italian_file_name" << "hangeul_file_name"
		<< "german_file_name" << "spanish_file_name" << "e-news_file_name" << "e-news-reread_file_name";

void CustomizeDialog::formats( QString course, QString& titleFormat, QString& fileNameFormat ) {
	int index = courses.indexOf( course );
	if ( index >= 0 ) {
		QString applicationDirPath = Utility::applicationDirPath();
		QSettings settings( applicationDirPath + INI_FILE, QSettings::IniFormat );
		settings.beginGroup( SETTING_GROUP );
		titleFormat = settings.value( titleKeys[index], DefaultTitle ).toString();
		fileNameFormat = settings.value( fileNameKeys[index], DefaultFileName ).toString();
		settings.endGroup();
	} else {
		titleFormat = DefaultTitle;
		fileNameFormat = DefaultFileName;
	}
}

CustomizeDialog::CustomizeDialog( Ui::DialogMode mode, QWidget *parent ) :
	QDialog(parent), mode(mode)
{
		ui.setupUi(this);

		this->setWindowTitle( mode == Ui::TitleMode ? QString::fromUtf8( "タイトルタグ設定" ) : QString::fromUtf8( "ファイル名設定" ) );
		settings( false );
		connect( this, SIGNAL( accepted() ), this, SLOT( accepted() ) );
}

void CustomizeDialog::settings( bool write ) {
	QLineEdit* lineEdits[] = {
		ui.lineEdit, ui.lineEdit_2, ui.lineEdit_3, ui.lineEdit_4,
		ui.lineEdit_5, ui.lineEdit_6, ui.lineEdit_7, ui.lineEdit_8,
		ui.lineEdit_9, ui.lineEdit_10, ui.lineEdit_11, ui.lineEdit_12,
		ui.lineEdit_13, ui.lineEdit_14, ui.lineEdit_15, ui.lineEdit_16,
		NULL
	};

	QString applicationDirPath = Utility::applicationDirPath();
	QSettings settings( applicationDirPath + INI_FILE, QSettings::IniFormat );
	settings.beginGroup( SETTING_GROUP );

	if ( !write ) {
		for ( int i = 0; lineEdits[i] != NULL; i++ ) {
			QString format = mode == Ui::TitleMode ?
							 settings.value( titleKeys[i], DefaultTitle ).toString() :
							 settings.value( fileNameKeys[i], DefaultFileName ).toString();
			lineEdits[i]->setText( format );
		}
	} else {
		for ( int i = 0; lineEdits[i] != NULL; i++ ) {
			QString text = lineEdits[i]->text();
			if ( mode == Ui::TitleMode )
				settings.setValue( titleKeys[i], text.length() == 0 ? DefaultTitle : text );
			else
				settings.setValue( fileNameKeys[i], text.length() == 0 ? DefaultFileName : text );
		}
	}
	settings.endGroup();
}

void CustomizeDialog::accepted() {
	settings( true );
}
