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
		<< QString::fromUtf8( "基礎英語3" ) << QString::fromUtf8( "英会話タイムトライアル" )
		<< QString::fromUtf8( "ラジオ英会話" ) << QString::fromUtf8( "入門ビジネス英語" )
		<< QString::fromUtf8( "実践ビジネス英語" ) << QString::fromUtf8( "まいにちロシア語" )
		<< QString::fromUtf8( "英語で読む村上春樹" )
		<< QString::fromUtf8( "まいにち中国語" ) << QString::fromUtf8( "まいにちフランス語" )
		<< QString::fromUtf8( "まいにちイタリア語" ) << QString::fromUtf8( "まいにちハングル講座" )
		<< QString::fromUtf8( "まいにちドイツ語" ) << QString::fromUtf8( "まいにちスペイン語" )
		<< QString::fromUtf8( "レベルアップ中国語" ) << QString::fromUtf8( "レベルアップハングル講座" )
		<< QString::fromUtf8( "攻略！英語リスニング" );
QStringList CustomizeDialog::titleKeys = QStringList()
		<< "basic1_title" << "basic2_title" << "basic3_title" << "timetrial_title"
		<< "kaiwa_title" << "business1_title" << "business2_title" << "russian_title"
		<< "yomu_title"
		<< "chinese_title" << "french_title" << "italian_title" << "hangeul_title"
		<< "german_title" << "spanish_title" << "levelup-chinese_title" << "levelup-hangeul_title"
		<< "kouryaku_title";
QStringList CustomizeDialog::fileNameKeys = QStringList()
		<< "basic1_file_name" << "basic2_file_name" << "basic3_file_name" << "timetrial_file_name"
		<< "kaiwa_file_name" << "business1_file_name" << "business2_file_name" << "russian_file_name"
		<< "yomu_file_name"
		<< "chinese_file_name" << "french_file_name" << "italian_file_name" << "hangeul_file_name"
		<< "german_file_name" << "spanish_file_name" << "levelup-chinese_file_name" << "levelup-hangeul_file_name"
		<< "kouryaku_file_name";

void CustomizeDialog::formats( QString course, QString& titleFormat, QString& fileNameFormat ) {
	int index = courses.indexOf( course );
	if ( index >= 0 ) {
		QString path = Utility::applicationBundlePath();
		QSettings settings( path + INI_FILE, QSettings::IniFormat );
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
		ui.lineEdit_17,
		ui.lineEdit_9, ui.lineEdit_10, ui.lineEdit_11, ui.lineEdit_12,
		ui.lineEdit_13, ui.lineEdit_14, ui.lineEdit_15, ui.lineEdit_16,
		ui.lineEdit_18,
		NULL
	};

	QString path = Utility::applicationBundlePath();
	QSettings settings( path + INI_FILE, QSettings::IniFormat );
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
