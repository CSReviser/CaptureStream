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
