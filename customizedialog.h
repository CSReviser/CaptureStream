/*
   Copyright (C) 2009-2011 jakago

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

#ifndef CUSTOMIZEDIALOG_H
#define CUSTOMIZEDIALOG_H

#include "ui_customizedialog.h"

namespace Ui {
	enum DialogMode {
		TitleMode, FileNameMode
	};
}

class CustomizeDialog : public QDialog {
	Q_OBJECT

public:
	explicit CustomizeDialog( Ui::DialogMode mode, QWidget *parent = 0 );
	static void formats( QString course, QString& titleFormat, QString& fileNameFormat );

signals:

public slots:
	void accepted();

private:
	Ui::CustomizeDialogClass ui;
	Ui::DialogMode mode;

	static QStringList courses;
	static QStringList titleKeys;
	static QStringList fileNameKeys;

	void settings( bool write );
};

#endif // CUSTOMIZEDIALOG_H
