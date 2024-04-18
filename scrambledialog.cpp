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

#include "scrambledialog.h"
#include "ui_scrambledialog.h"
#include "mainwindow.h"
#include "urldownloader.h"

#define OPTIONAL1 "7512_01"	// ニュースで学ぶ「現代英語」
#define OPTIONAL2 "0937_01"	// アラビア語講座
#define OPTIONAL3 "7629_01"	// Learn Japanese from the News
#define OPTIONAL4 "2769_01"	// ポルトガル語 ステップアップ

#define french1 "0953_01"
#define french2 "4412_01"
#define german1 "0943_01"
#define german2 "4410_01"
#define italian1 "0946_01"
#define italian2 "4411_01"
#define spanish1 "0948_01"
#define spanish2 "4413_01"
#define russian1 "0956_01"
#define russian2 "4414_01"
#define chinese1 "0915_01"
#define chinese2 "6581_01"
#define hangeul1 "0951_01"
#define hangeul2 "6810_01"

QString ScrambleDialog::optional1;
QString ScrambleDialog::optional2;
QString ScrambleDialog::optional3;
QString ScrambleDialog::optional4;

ScrambleDialog::ScrambleDialog( QString optional1, QString optional2, QString optional3, QString optional4, QWidget *parent )
//ScrambleDialog::ScrambleDialog( QString scramble, QWidget *parent )
		: QDialog(parent), ui(new Ui::ScrambleDialog) {
    ui->setupUi(this);
//	ui->scramble->setText( scramble );
	ui->optional1->setText( optional1 ),
	ui->optional2->setText( optional2 ),
	ui->optional3->setText( optional3 ),
	ui->optional4->setText( optional4 );
	ui->radioButton_9->setChecked(true);
}

ScrambleDialog::~ScrambleDialog() {
    delete ui;
}

//QString ScrambleDialog::scramble() {
//	return ui->scramble->text();
//}

QString ScrambleDialog::scramble1() {
	if (ui->radioButton->isChecked()) {
		optional1 = OPTIONAL1;
		ui->optional1->setText( optional1 );
	}
	if (ui->radioButton_1->isChecked()) {
		optional1 = "7155_01";
		ui->optional1->setText( optional1 );
	}
	if (ui->radioButton_2->isChecked()) {
		optional1 = "0164_01";
		ui->optional1->setText( optional1 );
	}
	if (ui->radioButton_3->isChecked()) {
		optional1 = "6311_01";
		ui->optional1->setText( optional1 );
	}
	if (ui->radioButton_4->isChecked()) {
		optional1 = "8367_01";
		ui->optional1->setText( optional1 );
	}
	if (ui->radioButton_5->isChecked()) {
		optional1 = "7155_01";
		ui->optional1->setText( optional1 );
	}
	return ui->optional1->text();
}

QString ScrambleDialog::scramble2() {
	if (ui->radioButton->isChecked()) {
		optional2 = OPTIONAL2;
		ui->optional2->setText( optional2 );
	}
	if (ui->radioButton_1->isChecked()) {
		optional2 = "0701_01";
		ui->optional2->setText( optional2 );
	}
	if (ui->radioButton_2->isChecked()) {
		optional2 = "0930_01";
		ui->optional2->setText( optional2 );
	}
	if (ui->radioButton_3->isChecked()) {
		optional2 = "1929_01";
		ui->optional2->setText( optional2 );
	}
	if (ui->radioButton_4->isChecked()) {
		optional2 = "0960_01";
		ui->optional2->setText( optional2 );
	}
	if (ui->radioButton_5->isChecked()) {
		optional2 = "0701_01";
		ui->optional2->setText( optional2 );
	}
	return ui->optional2->text();
}

QString ScrambleDialog::scramble3() {
	if (ui->radioButton->isChecked()) {
		optional3 = OPTIONAL3;
		ui->optional3->setText( optional3 );
	}
	if (ui->radioButton_1->isChecked()) {
		optional3 = "7629_01";
		ui->optional3->setText( optional3 );
	}
	if (ui->radioButton_2->isChecked()) {
		optional3 = "0058_01";
		ui->optional3->setText( optional3 );
	}
	if (ui->radioButton_3->isChecked()) {
		optional3 = "0961_01";
		ui->optional3->setText( optional3 );
	}
	if (ui->radioButton_4->isChecked()) {
		optional3 = "7412_01";
		ui->optional3->setText( optional3 );
	}
	if (ui->radioButton_5->isChecked()) {
		optional3 = "7629_01";
		ui->optional3->setText( optional3 );
	}
	return ui->optional3->text();
}

QString ScrambleDialog::scramble4() {
	if (ui->radioButton->isChecked()) {
		optional4 = OPTIONAL4;
		ui->optional4->setText( optional4 );
	}
	if (ui->radioButton_1->isChecked()) {
		optional4 = "7880_01";
		ui->optional4->setText( optional4 );
	}
	if (ui->radioButton_2->isChecked()) {
		optional4 = "7413_01";
		ui->optional4->setText( optional4 );
	}
	if (ui->radioButton_3->isChecked()) {
		optional4 = "3065_01";
		ui->optional4->setText( optional4 );
	}
	if (ui->radioButton_4->isChecked()) {
		optional4 = "0424_01";
		ui->optional4->setText( optional4 );
	}
	if (ui->radioButton_5->isChecked()) {
		optional4 = "0164_01";
		ui->optional4->setText( optional4 );
	}
	return ui->optional4->text();
}
