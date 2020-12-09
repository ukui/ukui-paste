/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mainframe.h"
#include "pasteitem.h"
#include "shortcut.h"
#include "searchbar.h"
#include "database.h"

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
#include <QDesktopWidget>
//#include <QtDBus>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QGSettings>


#define PANEL_DBUS_SERVICE "com.ukui.panel.desktop"
#define PANEL_DBUS_PATH "/"
#define PANEL_DBUS_INTERFACE "com.ukui.panel.desktop"
#define UKUI_PANEL_SETTING "org.ukui.panel.settings"
class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
	MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void desktoparea_change();
    void test();

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
    void initPanelDbusGsetting();
    int setClipBoardWidgetScaleFactor();
    int connectTaskBarDbus();
    int                         screenWidth;                                 // 屏幕分辨率的宽
    int                         screenHeight;                                // 屏幕分辨率的高
    QDBusInterface*             m_pServiceInterface;                            // 获取任务栏的高度
    QGSettings                  *m_pPanelSetting = nullptr;
    QDesktopWidget              *desktop;                                    // 桌面问题
    int desktop_width();
    int desktop_height();

public slots:
    void hide_window(void);                                                     //hide动画
    void show_window(void);                                                     //show动画
	void clipboard_later(void);
    void move_to_prev_next_focus_widget(bool);
    void desktoparea();
    void window_xy();
private:
    MainFrame               *__main_frame;
	QPropertyAnimation		*__hide_animation;
    DoubleCtrlShortcut		*__shortcut;
    bool                    __hide_state;
    Database                __db;

	/* widgets */
    SearchBar               *__searchbar;
    QListWidget             *__scroll_widget;

    QClipboard              *__clipboard;

	/* It's copyed from myself, We need save icon */
    QPixmap                 __pasteitem_icon;

	/* Use for store current row when searching */
    QListWidgetItem         *__current_item;
};
#endif // MAINWINDOW_H
