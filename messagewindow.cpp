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

#include "messagewindow.h"
#include "ui_messagewindow.h"
#include "mainwindow.h"
#include "utility.h"
#include "qt4qt5.h"

#include <QtGui>
#include <QTextCursor>
#include <QSettings>
#include <QVariant>

namespace {
	const QString SETTING_GROUP( "MessageWindow" );
	const QString SETTING_GEOMETRY( "geometry" );
	const int DEFAULT_WIDTH = 540;
	const int DEFAULT_HEIGHT = 300;
#ifdef QT4_QT5_WIN
	const int FONT_SIZE = 13;
#else
#ifdef QT4_QT5_MAC
	const int FONT_SIZE = 11;
#else
	const int FONT_SIZE = 14;
#endif
#endif
}

MessageWindow::MessageWindow(QWidget *parent) :
		QWidget(parent, Qt::CustomizeWindowHint|Qt::WindowTitleHint|Qt::WindowCloseButtonHint)/*, ui(new Ui::MessageWindow)*/ {
	//ui->setupUi(this);
	setupGui();
	settings( false );
}

MessageWindow::~MessageWindow() {
	if ( MainWindow::no_write_ini == "yes" )
	settings( true );
	//delete ui;
}

void MessageWindow::changeEvent( QEvent *e ) {
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
		//ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MessageWindow::setupGui() {
	textEdit = new QPlainTextEdit;
	textEdit->setReadOnly( true );
	textEdit->setWordWrapMode( QTextOption::WrapAnywhere );
	QFont* font = new QFont();
	font->setPixelSize( FONT_SIZE );
	textEdit->setFont( *font );

	clearTextButton = new QPushButton( QString::fromUtf8( "クリア" ) );
	connect( clearTextButton, SIGNAL(clicked()), this, SLOT(clearText()) );

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(textEdit);
	mainLayout->addWidget(clearTextButton);

	setLayout(mainLayout);
}

QString MessageWindow::text() {
	return textEdit->toPlainText();
}

// 改行あり
void MessageWindow::appendParagraph( const QString& text ) {
	if ( !Utility::nogui() ) {
		show();
		textEdit->appendPlainText( text );
	}
}

// 改行なし
void MessageWindow::append( const QString& text ) {
	if ( !Utility::nogui() ) {
		show();
		//textEdit->setPlainText( textEdit->toPlainText().append( text ) );	// これはとても遅い
		QTextCursor cursor = textEdit->textCursor();
		cursor.movePosition( QTextCursor::End );
		textEdit->setTextCursor( cursor );
		textEdit->insertPlainText( text );
	}
}

void MessageWindow::clearText() {
	textEdit->clear();
}

void MessageWindow::settings( bool write ) {
	QSettings settings( MainWindow::ini_file_path + INI_FILE, QSettings::IniFormat );
//#if defined( QT4_QT5_MAC ) || defined( QT4_QT5_WIN )
//	QSettings settings( Utility::applicationBundlePath() + INI_FILE, QSettings::IniFormat );
	settings.beginGroup( SETTING_GROUP );

	if ( !write ) {
		QVariant saved = settings.value( SETTING_GEOMETRY );
		if ( saved.type() != QVariant::Invalid )
			restoreGeometry( saved.toByteArray() );
		else
			resize( DEFAULT_WIDTH, DEFAULT_HEIGHT );
	} else {
		settings.setValue( SETTING_GEOMETRY, saveGeometry() );
	}

	settings.endGroup();
//#else
//	Q_UNUSED( write )
//	resize( DEFAULT_WIDTH, DEFAULT_HEIGHT );
//#endif
}
