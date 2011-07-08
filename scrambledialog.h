#ifndef SCRAMBLEDIALOG_H
#define SCRAMBLEDIALOG_H

#include <QDialog>

namespace Ui {
    class ScrambleDialog;
}

class ScrambleDialog : public QDialog {
    Q_OBJECT

public:
	explicit ScrambleDialog( QString scramble, QWidget *parent = 0 );
    ~ScrambleDialog();
	QString scramble();

private:
    Ui::ScrambleDialog *ui;
};

#endif // SCRAMBLEDIALOG_H
