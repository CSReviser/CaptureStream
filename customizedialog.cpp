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
#define DefaultTitle1 "%f"
#define DefaultTitle2 "%k_%Y_%M_%D"
#define DefaultFileName "%k_%Y_%M_%D"
#define DefaultFileName1 "%f_%Y_%M_%D"
#define DefaultFileName2 "%k_%Y-%M-%D"
#define DefaultFileName3 "%h"
#define DefaultFileName4 "%f"
#define DefaultFileName5 "%k_%h"

typedef struct LineEdit {
	QLineEdit* lineEdit;
	QString titleKey;
	QString fileNameKey;
} LineEdit;

QStringList CustomizeDialog::courses = QStringList()
		<< QString::fromUtf8( "json" ) << QString::fromUtf8( "xml" );
QStringList CustomizeDialog::titleKeys = QStringList()
		<< "customized_title1" << "customized_title2";
QStringList CustomizeDialog::fileNameKeys = QStringList()
		<< "customized_file_name1" << "customized_file_name2";
QStringList CustomizeDialog::titleDefaults = QStringList()
		<< DefaultTitle1 << DefaultTitle;
QStringList CustomizeDialog::fileNameDefaults = QStringList()
		<< DefaultFileName << DefaultFileName;
				
void CustomizeDialog::formats( QString course, QString& titleFormat, QString& fileNameFormat ) {
	int index = courses.indexOf( course );
	if ( index >= 0 ) {
		QString path = MainWindow::ini_file_path;
		QSettings settings( path + INI_FILE, QSettings::IniFormat );
		settings.beginGroup( SETTING_GROUP );
		titleFormat = settings.value( titleKeys[index], titleDefaults[index] ).toString();
		fileNameFormat = settings.value( fileNameKeys[index], fileNameDefaults[index] ).toString();
		settings.endGroup();
	} else {
		titleFormat = titleDefaults[index]; 
		fileNameFormat = fileNameDefaults[index];
	}
}

CustomizeDialog::CustomizeDialog( Ui::DialogMode mode, QWidget *parent ) :
	QDialog(parent), mode(mode)
{
		ui.setupUi(this);

		this->setWindowTitle( mode == Ui::TitleMode ? QString::fromUtf8( "タイトルタグ設定" ) : QString::fromUtf8( "ファイル名設定" ) );
		settings( false );
		connect( this, SIGNAL( accepted() ), this, SLOT( accepted() ) );
		ui.radioButton_9->setChecked(true);
		ui.radioButton_19->setChecked(true);
}

void CustomizeDialog::settings( bool write ) {
 	if (ui.radioButton->isChecked()) ui.lineEdit->setText( DefaultFileName );
 	if (ui.radioButton_1->isChecked()) ui.lineEdit->setText( DefaultFileName1 );
  	if (ui.radioButton_2->isChecked()) ui.lineEdit->setText( DefaultFileName2 );
  	if (ui.radioButton_3->isChecked()) ui.lineEdit->setText( DefaultFileName3 );
  	if (ui.radioButton_4->isChecked()) ui.lineEdit->setText( DefaultFileName4 );
	if (ui.radioButton_5->isChecked()) ui.lineEdit->setText( DefaultFileName5 );
  	if (ui.radioButton_10->isChecked()) ui.lineEdit_2->setText( DefaultFileName );

	QLineEdit* lineEdits[] = {
		ui.lineEdit, ui.lineEdit_2,
		NULL
	};
	
	QString path =  MainWindow::ini_file_path;
	QSettings settings( path + INI_FILE, QSettings::IniFormat );
	settings.beginGroup( SETTING_GROUP );
	
	if ( !write ) {
		for ( int i = 0; lineEdits[i] != NULL; i++ ) {
			QString format = mode == Ui::TitleMode ?
							 settings.value( titleKeys[i], titleDefaults[i] ).toString() :
							 settings.value( fileNameKeys[i], fileNameDefaults[i] ).toString();
			lineEdits[i]->setText( format );
		}
	} else {
		for ( int i = 0; lineEdits[i] != NULL; i++ ) {
			QString text = lineEdits[i]->text();
			if ( mode == Ui::TitleMode ) 
				settings.setValue( titleKeys[i], text.length() == 0 ? titleDefaults[i] : text );
			else 
				settings.setValue( fileNameKeys[i], text.length() == 0 ? fileNameDefaults[i] : text );
		}
	}
	settings.endGroup();
}

void CustomizeDialog::accepted() {
	if ( MainWindow::no_write_ini == "yes" )
	settings( true );
}
