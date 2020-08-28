#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "3rd/qxtglobalshortcut5/gui/qxtglobalshortcut.h"

#include <QMainWindow>
#include <QFrame>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

protected:
	void focusOutEvent(QFocusEvent *e);

private:
	QFrame				*__main_frame;
	QGraphicsDropShadowEffect	*__main_frame_shadow;
	QPropertyAnimation		*__hide_animation;
	QxtGlobalShortcut		*__shortcut;
};
#endif // MAINWINDOW_H
