#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "3rd/qxtglobalshortcut5/gui/qxtglobalshortcut.h"
#include "pasteitem.h"

#include <QMainWindow>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QListWidget>
#include <QListWidgetItem>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

protected:
	bool event(QEvent *e);
	void showEvent(QShowEvent *event);

private:
	void initUI(void);
	QWidget *createItemWidget();
	static void loadStyleSheet(QWidget *, const QString &);

public Q_SLOTS:
	void hide_window(void);
	void show_window(void);

private:
	QWidget				*__main_frame;
	QGraphicsDropShadowEffect	*__main_frame_shadow;
	QPropertyAnimation		*__hide_animation;
	QxtGlobalShortcut		*__shortcut;

	/* widgets */
	QHBoxLayout			*__hlayout;
	QVBoxLayout			*__vlayout;
	QListWidget			*__scroll_widget;
};
#endif // MAINWINDOW_H
