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
