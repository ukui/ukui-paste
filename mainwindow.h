#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

#include "pasteitem.h"
#include "shortcut.h"
#include "searchbar.h"
#include "database.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

protected:
	bool event(QEvent *e);
	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event);

private:
	void initUI(void);
	void reloadData(void);
	PasteItem *insertItemWidget(void);
	void resetItemTabOrder(void);
	static void loadStyleSheet(QWidget *, const QString &);
	QPixmap getClipboardOwnerIcon(void);
	void enabledGlassEffect(void);

public Q_SLOTS:
	void hide_window(void);
	void show_window(void);
	void clipboard_later(void);

private:
	QWidget				*__main_frame;
	QGraphicsDropShadowEffect	*__main_frame_shadow;
	QPropertyAnimation		*__hide_animation;
	DoubleCtrlShortcut		*__shortcut;
	/* That is a workaround for hide window */
	bool				__hide_state;
	Database			__db;

	/* widgets */
	SearchBar			*__searchbar;
	PushButton			*__menu_button;
	QListWidget			*__scroll_widget;

	QClipboard			*__clipboard;

	/* It's copyed from myself, We need save icon */
	QPixmap				__pasteitem_icon;

	/* Use for store current row when searching */
	QListWidgetItem			*__current_item;
};
#endif // MAINWINDOW_H
