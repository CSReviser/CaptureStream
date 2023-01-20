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

QString ScrambleDialog::optional1;
QString ScrambleDialog::optional2;
QString ScrambleDialog::optional3;
QString ScrambleDialog::optional4;


QString ScrambleDialog::opt1[] = {
		"7155", //Living in Japan
		"0937", //アラビア語講座
		"0701", //やさしい日本語
		"7629"  //Learn Japanese from the News
};

QString ScrambleDialog::opt2[] = {
		"0953", //まいにちフランス語 入門編
		"0943", //まいにちドイツ語 入門編／初級編
		"0946", //まいにちイタリア語 入門編
		"0948"  //まいにちスペイン語 入門編／初級編
};
QString ScrambleDialog::opt3[] = {
		"0956", //まいにちロシア語 入門編
		"1893", //ポルトガル語講座 入門
		"0915", //まいにち中国語
		"0951"  //まいにちハングル講座
};
QString ScrambleDialog::opt4[] = {
		"4412", //まいにちフランス語 応用編
		"4410", //まいにちドイツ語 応用編
		"4411", //まいにちイタリア語 応用編
		"4413"  //まいにちスペイン語 中級編／応用編
};
QString ScrambleDialog::opt5[] = {
		"4414", //まいにちロシア語 応用編
		"2769", //ポルトガル語ステップアップ
		"6581", //ステップアップ中国語
		"6810"  //ステップアップ ハングル講座
};

QString ScrambleDialog::opt6[] = {
		"0164", //青春アドベンチャー
		"0930", //新日曜名作座
		"8062", //朗読
		"0058"  //FMシアター
};

ScrambleDialog::ScrambleDialog( QString optional1, QString optional2, QString optional3, QString optional4, QWidget *parent )
//ScrambleDialog::ScrambleDialog( QString optional1, QString optional2, QString optional3, QString optional4, QString optional5, QString optional6, QString optional7, QString optional8, QWidget *parent )
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
	if (ui->radioButton->isChecked())	optional1 = opt1[0];
	if (ui->radioButton_1->isChecked()) 	optional1 = opt2[0];
	if (ui->radioButton_2->isChecked()) 	optional1 = opt3[0];
	if (ui->radioButton_3->isChecked()) 	optional1 = opt4[0];
	if (ui->radioButton_4->isChecked()) 	optional1 = opt5[0];
	if (ui->radioButton_5->isChecked()) 	optional1 = opt6[0];
	if (!(ui->radioButton_9->isChecked())) 	ui->optional1->setText( optional1 );
	return ui->optional1->text();
}

QString ScrambleDialog::scramble2() {
	if (ui->radioButton->isChecked()) 	optional2 = opt1[1];
	if (ui->radioButton_1->isChecked()) 	optional2 = opt2[1];
	if (ui->radioButton_2->isChecked()) 	optional2 = opt3[1];
	if (ui->radioButton_3->isChecked())	optional2 = opt4[1];
	if (ui->radioButton_4->isChecked()) 	optional2 = opt5[1];
	if (ui->radioButton_5->isChecked()) 	optional2 = opt6[1];
	if (!(ui->radioButton_9->isChecked())) 	ui->optional2->setText( optional2 );
	return ui->optional2->text();
}

QString ScrambleDialog::scramble3() {
	if (ui->radioButton->isChecked()) 	optional3 = opt1[2];
	if (ui->radioButton_1->isChecked()) 	optional3 = opt2[2];
	if (ui->radioButton_2->isChecked()) 	optional3 = opt3[2];
	if (ui->radioButton_3->isChecked()) 	optional3 = opt4[2];
	if (ui->radioButton_4->isChecked()) 	optional3 = opt5[2];
	if (ui->radioButton_5->isChecked()) 	optional3 = opt6[2];
	if (!(ui->radioButton_9->isChecked())) 	ui->optional3->setText( optional3 );
	return ui->optional3->text();
}

QString ScrambleDialog::scramble4() {
	if (ui->radioButton->isChecked()) 	optional4 = opt1[3];
	if (ui->radioButton_1->isChecked()) 	optional4 = opt2[3];
	if (ui->radioButton_2->isChecked()) 	optional4 = opt3[3];
	if (ui->radioButton_3->isChecked()) 	optional4 = opt4[3];
	if (ui->radioButton_4->isChecked()) 	optional4 = opt5[3];
	if (ui->radioButton_5->isChecked()) 	optional4 = opt6[3];
	if (!(ui->radioButton_9->isChecked())) 	ui->optional4->setText( optional4 );
	return ui->optional4->text();
}
