#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "3rd/qxtglobalshortcut5/gui/qxtglobalshortcut.h"
#include "pasteitem.h"
#include "shortcut.h"

#include <QMainWindow>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QListWidget>
#include <QListWidgetItem>
#include <QClipboard>
#include <QTimer>

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
	PasteItem *insertItemWidget(QListWidgetItem **);
	void resetItemTabOrder(void);
	static void loadStyleSheet(QWidget *, const QString &);
	QPixmap getClipboardOwnerIcon(void);
	void enabledGlassEffect(void);
	bool isMeTrigger(void);

public Q_SLOTS:
	void hide_window(void);
	void show_window(void);
	void clipboard_later(void);

private:
	QWidget				*__main_frame;
	QGraphicsDropShadowEffect	*__main_frame_shadow;
	QPropertyAnimation		*__hide_animation;
//	QxtGlobalShortcut		*__shortcut;
	Shortcut			*__shortcut;
	/* That is a workaround for hide window */
	bool				__hide_state;

	/* widgets */
	QHBoxLayout			*__hlayout;
	QVBoxLayout			*__vlayout;
	QListWidget			*__scroll_widget;

	QClipboard			*__clipboard;

	bool				__is_me_trigger;
};
#endif // MAINWINDOW_H
