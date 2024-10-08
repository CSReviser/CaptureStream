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
#include "utility.h"
#include "downloadthread.h"

QString ScrambleDialog::optional1;
QString ScrambleDialog::optional2;
QString ScrambleDialog::optional3;
QString ScrambleDialog::optional4;


QString ScrambleDialog::opt1[] = {
		"77RQWQX1L6_01", // ニュースで学ぶ「現代英語」
		"WKMNWGMN6R_01", // アラビア語講座
		"GLZQ4M519X_01", // Asian View
		"N13V9K157Y_01"  // ポルトガル語講座
};
QString ScrambleDialog::opt2[] = {
		"XQ487ZM61K_x1", //まいにちフランス語 入門編／初級編
		"N8PZRZ9WQY_x1", //まいにちドイツ語 入門編／初級編
		"LJWZP7XVMX_x1", //まいにちイタリア語 入門編／初級編
		"NRZWXVGQ19_x1"  //まいにちスペイン語 入門編／初級編
};
QString ScrambleDialog::opt3[] = {
		"XQ487ZM61K_y1", //まいにちフランス語 応用編
		"N8PZRZ9WQY_y1", //まいにちドイツ語 応用編
		"LJWZP7XVMX_y1", //まいにちイタリア語 応用編
		"NRZWXVGQ19_y1"  //まいにちスペイン語 応用編
};
QString ScrambleDialog::opt4[] = {
		"YRLK72JZ7Q_x1", //まいにちロシア語 入門編／初級編
		"YRLK72JZ7Q_y1", //まいにちロシア語 応用編
		"4MY6Q8XP88_01", //Living in Japan
		"6LPPKP6W8Q_01", //やさしい日本語
};
QString ScrambleDialog::opt5[] = {
		"R5XR783QK3_01", //おしゃべりな古典教室
		"DK83KZ8848_01", //カルチャーラジオ 文学の世界
		"5L3859P515_01", //古典講読
		"XKR4W8GY15_01"  //カルチャーラジオ 科学と人間
};
QString ScrambleDialog::opt6[] = {
		"X4X6N1XG8Z_01", //青春アドベンチャー
		"D85RZVGX7W_01", //新日曜名作座
		"LRK2VXPK5X_01", //朗読
		"M65G6QLKMY_01"  //FMシアター
};

QString ScrambleDialog::opt7[] = {
		"4K58V66ZGQ_01", //梶裕貴のラジオ劇場
		"X78J5NKWM9_01", //こころをよむ
		"MVYJ6PRZMX_01", //アナウンサー百年百話
		"JWQ88ZVWQK_01"  //宗教の時間
};


ScrambleDialog::ScrambleDialog( QString optional1, QString optional2, QString optional3, QString optional4, QWidget *parent )
//ScrambleDialog::ScrambleDialog( QString scramble, QWidget *parent )
		: QDialog(parent), ui(new Ui::ScrambleDialog) {
    ui->setupUi(this);
	QString optional[] = { optional1, optional2, optional3, optional4 };
	QLineEdit*  Button2[] = { ui->optional1, ui->optional2, ui->optional3, ui->optional4 };
	for ( int i = 0 ; i < 4 ; ++i ) Button2[i]->setText( optional[i] );
	ui->radioButton_9->setChecked(true);
}

ScrambleDialog::~ScrambleDialog() {
    delete ui;
}

QString ScrambleDialog::scramble_set( QString opt, int i ) {
	QString opt_tmp = opt;
	QString optional[] = { optional1, optional2, optional3, optional4 };
	QString opt_set[] = { opt1[i], opt2[i], opt3[i], opt4[i], opt5[i], opt6[i], opt7[i]  };
	QAbstractButton*  Button[] = { ui->radioButton, ui->radioButton_1, ui->radioButton_2, ui->radioButton_3, ui->radioButton_4, ui->radioButton_5, NULL };
	QLineEdit*  Button2[] = { ui->optional1, ui->optional2, ui->optional3, ui->optional4, NULL };
	for ( int j = 0 ; Button[j] != NULL ; j++ ) 
		if (Button[j]->isChecked())	opt = opt_set[j];
	if (!(ui->radioButton_9->isChecked())) Button2[i]->setText( opt );
	if ( ui->radioButton_9->isChecked() && Utility::getProgram_name( Button2[i]->text() ) == "" ) { Button2[i]->setText( opt ); }
	return opt;
}
QString ScrambleDialog::scramble1() {
	optional1 = scramble_set( optional1, 0);
	return ui->optional1->text();
}
QString ScrambleDialog::scramble2() {
	optional2 = scramble_set( optional2, 1 );
	return ui->optional2->text();
}
QString ScrambleDialog::scramble3() {
	optional3 = scramble_set( optional3, 2 );
	return ui->optional3->text();
}
QString ScrambleDialog::scramble4() {
	optional4 = scramble_set( optional4, 3 );
	return ui->optional4->text();
}


