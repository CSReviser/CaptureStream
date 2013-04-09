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

#ifndef MESSAGEWINDOW_H
#define MESSAGEWINDOW_H

#include <QWidget>
#include <QString>
#include <QPlainTextEdit>
#include <QPushButton>

namespace Ui {
    class MessageWindow;
}

class MessageWindow : public QWidget {
    Q_OBJECT

public:
	MessageWindow( QWidget *parent = 0 );
    ~MessageWindow();
	QString text();

public slots:
	void appendParagraph( const QString& text );
	void append( const QString& text );

protected:
    void changeEvent(QEvent *e);

private slots:
	void clearText();

private:
	//Ui::MessageWindow *ui;
	QPlainTextEdit *textEdit;
	QPushButton *clearTextButton;

	void setupGui();
	void settings( bool write );
};

#endif // MESSAGEWINDOW_H
