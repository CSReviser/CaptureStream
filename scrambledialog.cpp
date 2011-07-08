#include "scrambledialog.h"
#include "ui_scrambledialog.h"

ScrambleDialog::ScrambleDialog( QString scramble, QWidget *parent )
		: QDialog(parent), ui(new Ui::ScrambleDialog) {
    ui->setupUi(this);
	ui->scramble->setText( scramble );
}

ScrambleDialog::~ScrambleDialog() {
    delete ui;
}

QString ScrambleDialog::scramble() {
	return ui->scramble->text();
}
