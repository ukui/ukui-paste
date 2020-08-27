#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPropertyAnimation>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

protected:
	void focusOutEvent(QFocusEvent *e);

private:
	QPropertyAnimation *__hide_animation;
};
#endif // MAINWINDOW_H
