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

#include <QApplication>
#include <QScreen>
#include <QLabel>
#include <QSizePolicy>
#include <QScroller>
#include <QFile>
#include <QMessageBox>
#include <QMimeData>
#include <QCryptographicHash>
#include <QImage>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QUrl>
#include <QBuffer>
#include <QMenu>
#include <QAction>
#include <QMutex>
#include <QMutexLocker>
#include <QShortcut>
#include <QEvent>
#include <QDebug>

#include "mainwindow.h"

#ifdef Q_OS_WIN
#include <QtWin>
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <shellapi.h>
#include <comdef.h>
#include <commctrl.h>
#include <objbase.h>
#include <commoncontrols.h>
#include <psapi.h>
#include <QOperatingSystemVersion>
#endif

#ifdef Q_OS_LINUX
#include <KF5/KWindowSystem/KWindowEffects>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

#ifdef Q_OS_WIN
typedef enum _WINDOWCOMPOSITIONATTRIB
{
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_NCPAINT = 4,
    WCA_CAPTION_BUTTON_BOUNDS = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_FORCE_ICONIC_REPRESENTATION = 7,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_HAS_ICONIC_BITMAP = 9,
    WCA_THEME_ATTRIBUTES = 10,
    WCA_NCRENDERING_EXILED = 11,
    WCA_NCADORNMENTINFO = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
    WCA_VIDEO_OVERLAY_ACTIVE = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK = 16,
    WCA_CLOAK = 17,
    WCA_CLOAKED = 18,
    WCA_ACCENT_POLICY = 19,
    WCA_FREEZE_REPRESENTATION = 20,
    WCA_EVER_UNCLOAKED = 21,
    WCA_VISUAL_OWNER = 22,
    WCA_LAST = 23
} WINDOWCOMPOSITIONATTRIB;

typedef struct _WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
} WINDOWCOMPOSITIONATTRIBDATA;

typedef enum _ACCENT_STATE
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_INVALID_STATE = 4
} ACCENT_STATE;

typedef struct _ACCENT_POLICY
{
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
} ACCENT_POLICY;

typedef BOOL (WINAPI *pfnSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);
#endif
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      __main_frame(new MainFrame(this)),
      __hide_animation(new QPropertyAnimation(this, "pos")),
      __shortcut(new DoubleCtrlShortcut(this)),
      __hide_state(true),
      __clipboard(QApplication::clipboard()),
      __current_item(nullptr)
{
    /* 初始化与任务栏交互的dbus和gsetting */
//    initPanelDbusGsetting();

    /*获取分辨率*/
    desktop_width();
    desktop_height();
    /*整体窗口位置*/
    window_xy();
    /*分辨率是否变化*/
    desktoparea_change();

    QObject::connect(this->__clipboard, &QClipboard::dataChanged, [this](void) {
        QTimer::singleShot(100, this, SLOT(clipboard_later()));
    });
    QObject::connect(this->__hide_animation, &QPropertyAnimation::finished, [this](void) {
        if (this->__hide_animation->direction() == QAbstractAnimation::Forward) {
            /* Hidden stage */
            this->hide();
        }
    });
    this->__hide_animation->setDuration(200);
    this->__hide_animation->setStartValue(this->pos());
    this->__hide_animation->setEndValue(QPoint(0, QApplication::primaryScreen()->geometry().height()));
    this->__hide_animation->setEasingCurve(QEasingCurve::OutQuad);

    QObject::connect(this->__shortcut, &DoubleCtrlShortcut::activated, [this](void) {
        if (!this->isVisible())
            this->show_window();
        else
            this->hide_window();
    });

    QShortcut *shortcut_search = new QShortcut(this);
    shortcut_search->setKey(QKeySequence("Ctrl+f"));
    QObject::connect(shortcut_search, &QShortcut::activated, [this](void) {
        LineEdit *lineedit = this->__searchbar->findChild<LineEdit *>("", Qt::FindDirectChildrenOnly);
        lineedit->setFocus();
    });

    this->initUI();
    return;
}

//void MainWindow::initPanelDbusGsetting()
//{
//    /* 链接任务栏Dbus接口，获取任务栏高度和位置 */
//    m_pServiceInterface = new QDBusInterface(PANEL_DBUS_SERVICE, PANEL_DBUS_PATH, PANEL_DBUS_INTERFACE, QDBusConnection::sessionBus());
//    m_pServiceInterface->setTimeout(2147483647);

//    /* 链接任务栏dgsetting接口 */
//    if(QGSettings::isSchemaInstalled(UKUI_PANEL_SETTING))
//        m_pPanelSetting = new QGSettings(UKUI_PANEL_SETTING);
//}

/*整体窗口位置*/
void MainWindow::window_xy(){
        this->setGeometry(8, screenHeight*0.5, screenWidth-16, screenHeight*0.2);
        qDebug() <<screenHeight*0.5;
        this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                     Qt::BypassWindowManagerHint | Qt::SplashScreen);
        this->setFocusPolicy(Qt::NoFocus);
        this->setCentralWidget(this->__main_frame);
        this->__main_frame->setFocusPolicy(Qt::ClickFocus);
        QObject::connect(this->__main_frame, SIGNAL(moveFocusPrevNext(bool)), this, SLOT(move_to_prev_next_focus_widget(bool)));
}
/*获取桌面是否有改变*/
void MainWindow::desktoparea_change(){
    QDesktopWidget* desktopWidget = QApplication::desktop();
    connect(desktopWidget, &QDesktopWidget::resized, this, &MainWindow::desktoparea);
}
void MainWindow::desktoparea(){
    desktop_width();
    desktop_height();
    window_xy();
}
/*获取主桌面分辨率大小*/
int MainWindow::desktop_width(){
    QRect screenRect1 = desktop->screenGeometry();
    screenWidth = screenRect1.width();
    qDebug() << "主屏幕宽" << screenWidth;
    return screenWidth;
}
int MainWindow::desktop_height(){
    QRect screenRect2 = desktop->screenGeometry();
    screenHeight = screenRect2.height();
    qDebug() << "主屏幕高" << screenHeight;
    return screenHeight;
}

/*获取任务栏状态是否有改变*/
/*获取任务栏的大小*/
//int MainWindow::connectTaskBarDbus()
//{
//    int panelHeight = 46;
//    if (m_pPanelSetting != nullptr) {
//        QStringList keys = m_pPanelSetting->keys();
//        if (keys.contains("panelsize")) {
//            panelHeight = m_pPanelSetting->get("panelsize").toInt();
//        }
//    } else {
//        QDBusMessage msg = m_pServiceInterface->call("GetPanelSize", QVariant("Hight"));
//        panelHeight = msg.arguments().at(0).toInt();
//        return panelHeight;
//    }
//    return panelHeight;
//}
/*获取任务栏位置*/
//int MainWindow::getPanelSite()
//{
//    int panelPosition = 0;
//    if (m_pPanelSetting != nullptr) {
//        QStringList keys = m_pPanelSetting->keys();
//        if (keys.contains("panelposition")) {
//            panelPosition = m_pPanelSetting->get("panelposition").toInt();
//        }
//    } else {
//        QDBusMessage msg = m_pServiceInterface->call("GetPanelPosition", QVariant("Site"));
//        panelPosition = msg.arguments().at(0).toInt();
//    }
//    qDebug() << "panel所在的位置" << panelPosition;
//    return panelPosition;
//}
/* 修改屏幕分辨率或者主屏需要做的事情 */
//void MainWindow::ModifyScreenNeeds()
//{
//    int clipboardhight = setClipBoardWidgetScaleFactor();
//    sidebarPluginsWidgets::getInstancePluinsWidgets()->setClipboardWidgetSize(clipboardhight); //设定剪贴板高度
//    return;
//}
/* 设定剪贴板高度 */
//int MainWindow::setClipBoardWidgetScaleFactor()
//{
//    if (screenHeight >= 600 && screenHeight <= 768) {
//        return screenHeight/2 - connectTaskBarDbus() - 60;
//    } else if (screenHeight >= 900 && screenHeight <= 1080) {
//        return screenHeight/3;
//    } else if (screenHeight >= 1200 && screenHeight <= 2160) {
//        return screenHeight/4;
//    } else {
//        return screenHeight/2 - connectTaskBarDbus();
//    }
//}
/* 设置剪贴板的高度 */
//void sidebarPluginsWidgets::setClipboardWidgetSize(int ClipHight)
//{
//    m_cliboardHight = ClipHight;
//    qDebug() << "设置小剪贴板的界面大小---->" << ClipHight;
//    this->setFixedSize(400, ClipHight);
//    m_pClipboardWidget->setFixedSize(400, ClipHight - 50);
//    m_pPluginsButtonWidget->setFixedSize(400, ClipHight - 50);
//    return;
//}
//链接任务栏dbus获取高度的接口
//int MainWindow::connectTaskBarDbus()
//{
//    int panelHeight = 46;
//    if (m_pPanelSetting != nullptr) {
//        QStringList keys = m_pPanelSetting->keys();
//        if (keys.contains("panelsize")) {
//            panelHeight = m_pPanelSetting->get("panelsize").toInt();
//        }
//    } else {
//        QDBusMessage msg = m_pServiceInterface->call("GetPanelSize", QVariant("Hight"));
//        panelHeight = msg.arguments().at(0).toInt();
//        return panelHeight;
//    }
//    return panelHeight;
//}

MainWindow::~MainWindow()
{
    delete this->__hide_animation;
}
bool MainWindow::event(QEvent *e)
{
    if (e->type() == QEvent::ActivationChange) {
        if (QApplication::activeWindow() != this)
            this->hide_window();
    }

    return QMainWindow::event(e);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QListWidgetItem *item = this->__scroll_widget->currentItem();
    if(item) {
        auto *widget = this->__scroll_widget->itemWidget(item);
        /* Let the selected item has focus */
        widget->setFocus();
        this->__scroll_widget->scrollToItem(item);
    }

    /* That is a workaround for QListWidget pixel scroll */
    int count = this->__scroll_widget->count();
    if (count > 0) {
        this->__scroll_widget->item(count-1)->setHidden(false);
    }

    QWidget::showEvent(event);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    /* That is a workaround for QListWidget pixel scroll */
    int count = this->__scroll_widget->count();
    if(count > 0) {
        this->__scroll_widget->item(count-1)->setHidden(true);
    }

    QWidget::hideEvent(event);
}

void MainWindow::show_window(void)
{
    this->__hide_animation->setDirection(QAbstractAnimation::Backward);
    this->__hide_animation->start();
    this->__hide_state = false;
    this->activateWindow();
    this->show();
}

void MainWindow::hide_window(void)
{
    if (this->__hide_state)
        return;

    this->__hide_animation->setDirection(QAbstractAnimation::Forward);
    this->__hide_animation->start();
    this->__hide_state = true;
}

void MainWindow::move_to_prev_next_focus_widget(bool prev)
{
    QListWidgetItem *item = this->__scroll_widget->selectedItems()[0];
    PasteItem *widget = reinterpret_cast<PasteItem *>(this->__scroll_widget->itemWidget(item));

    do {
        if (prev) {
            /* Get prev focus widget and isn't hidden */
            widget = reinterpret_cast<PasteItem *>(widget->previousInFocusChain());
        } else {
            /* Get next focus widget and isn't hidden */
            widget = reinterpret_cast<PasteItem *>(widget->nextInFocusChain());
        }
    } while (widget->isHidden());
    widget->setFocus();
}

void MainWindow::initUI(void)
{
    this->__searchbar = new SearchBar(this->__main_frame, this->width()/3, 40);
    QObject::connect(this->__searchbar, &SearchBar::textChanged, [this](const QString &text) {
        int temp_current_item_row = -1;
        int show_row_count = 0;

        /* Store current row num when first time searching */
        if (this->__current_item == nullptr) {
            this->__current_item = this->__scroll_widget->currentItem();
        }

        for (int i = 0; i < this->__scroll_widget->count(); i++) {
            QListWidgetItem *item = this->__scroll_widget->item(i);
            PasteItem *widget = reinterpret_cast<PasteItem *>(this->__scroll_widget->itemWidget(item));
            if (!widget->text().toLower().contains(text.toLower())) {
                item->setHidden(true);
            } else {
                item->setHidden(false);
                show_row_count++;

                if (temp_current_item_row == -1) {
                    temp_current_item_row = i;
                }
            }
        }

        /* That is the first showing item */
        if (temp_current_item_row != -1) {
            this->__scroll_widget->item(temp_current_item_row)->setSelected(true);
            this->__scroll_widget->scrollToItem(this->__scroll_widget->item(temp_current_item_row));
        }

        if (show_row_count == this->__scroll_widget->count()) {
            /* restore current row in search before */
            this->__current_item->setSelected(true);
            this->__scroll_widget->scrollToItem(this->__current_item);
            this->__current_item = nullptr;
        }
    });
    QObject::connect(this->__searchbar, &SearchBar::selectItem, [this](void) {
        QListWidgetItem *item = this->__scroll_widget->selectedItems()[0];
        PasteItem *widget = reinterpret_cast<PasteItem *>(this->__scroll_widget->itemWidget(item));
        this->__current_item = nullptr;
        widget->copyData();
    });
    QObject::connect(this->__searchbar, SIGNAL(moveFocusPrevNext(bool)), this, SLOT(move_to_prev_next_focus_widget(bool)));


    this->__scroll_widget = new QListWidget(this->__main_frame);
    this->__scroll_widget->setSelectionMode(QAbstractItemView::SingleSelection);
    this->__scroll_widget->setHorizontalScrollMode(QListWidget::ScrollPerPixel);
    this->__scroll_widget->setFlow(QListView::LeftToRight);
    this->__scroll_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->__scroll_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->__scroll_widget->setViewMode(QListView::ListMode);
    this->__scroll_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->__scroll_widget->setFrameShape(QListWidget::NoFrame);
    this->__scroll_widget->setSpacing(10);
    this->__scroll_widget->setWrapping(false);
    this->__scroll_widget->setFocusPolicy(Qt::NoFocus);
    QScroller::grabGesture(this->__scroll_widget, QScroller::LeftMouseButtonGesture);
    QObject::connect(this->__scroll_widget, &QListWidget::currentItemChanged, [this](void) {
        this->__scroll_widget->update();
    });
    /*搜索栏水平排列*/
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(this->__searchbar);
    hlayout->addStretch();

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addLayout(hlayout);
    vlayout->addWidget(this->__scroll_widget);

    this->__main_frame->setLayout(vlayout);
    /* need this for resize this->__scroll_widget size */
    this->__main_frame->show();

    /* load data from database */
    this->reloadData();
}

void MainWindow::reloadData()
{
    if (!this->__db.isTableExist()) {
        this->__db.createTable();
        return;
    }

    for (auto itemData : this->__db.loadData()) {
        /* remove the data if it's too old (than a week) */
        if (QDateTime::currentDateTime().toSecsSinceEpoch() - itemData->time.toSecsSinceEpoch() > (60 * 60 * 24 * 7)) {
            this->__db.delelePasteItem(itemData->md5);
            continue;
        }

        PasteItem *widget = this->insertItemWidget();

        if (itemData->mimeData->hasHtml() && !itemData->mimeData->text().isEmpty()) {
            widget->setRichText(itemData->mimeData->html(), itemData->mimeData->text());
        } else if (itemData->mimeData->hasImage()) {
            widget->setImage(itemData->image);
        } else if (itemData->mimeData->hasUrls()) {
            QList<QUrl> urls = itemData->mimeData->urls();
            if (!widget->setUrls(urls)) {
                this->__db.delelePasteItem(itemData->md5);
                delete this->__scroll_widget->takeItem(0);
                continue;
            }
        } else if (itemData->mimeData->hasText() && !itemData->mimeData->text().isEmpty()) {
            widget->setPlainText(itemData->mimeData->text().trimmed());
        } else {
            /* No data, remove it */
            delete this->__scroll_widget->takeItem(0);
            continue;
        }
        widget->setTime(itemData->time);
        widget->setIcon(itemData->icon);

        this->__scroll_widget->item(0)->setData(Qt::UserRole, QVariant::fromValue(*itemData));
    }
}


/* Insert a PasteItem into listwidget */
PasteItem *MainWindow::insertItemWidget(void)
{
    QListWidgetItem *item = new QListWidgetItem;
    auto *widget = new PasteItem(nullptr, item);

    QObject::connect(widget, &PasteItem::hideWindow, [this](void) {
        this->hide_window();
    });

    QRect rect = QApplication::primaryScreen()->geometry();
    /* resize item, It's use for pasteitem frame */
    item->setSizeHint(QSize(rect.width()/6, 1));

    this->__scroll_widget->insertItem(0, item);
    this->__scroll_widget->setCurrentRow(0);
    this->__scroll_widget->setItemWidget(item, widget);
    this->resetItemTabOrder();

    return widget;
}

void MainWindow::resetItemTabOrder(void)
{
    int count = this->__scroll_widget->count();

    for (int i = 0; i < count-1; i++) {
        QWidget *first = this->__scroll_widget->itemWidget(this->__scroll_widget->item(i));
        QWidget *second = this->__scroll_widget->itemWidget(this->__scroll_widget->item(i+1));
        QWidget::setTabOrder(first, second);
    }
}

QMutex clipboard_mutex;
void MainWindow::clipboard_later(void)
{
    QMutexLocker locker(&clipboard_mutex);
    const QMimeData *mime_data = this->__clipboard->mimeData();
    PasteItem *widget = nullptr;
    QByteArray md5_data;
    ItemData itemData;
    itemData.mimeData = new QMimeData;

    foreach (QString format, mime_data->formats()) {
        itemData.mimeData->setData(format, mime_data->data(format));
    }
    if (mime_data->hasImage()) {
        itemData.image = qvariant_cast<QImage>(mime_data->imageData());
    }

    widget = this->insertItemWidget();

    if (itemData.mimeData->hasHtml() && !itemData.mimeData->text().isEmpty()) {
        widget->setRichText(itemData.mimeData->html(), itemData.mimeData->text());
        md5_data = itemData.mimeData->text().toLocal8Bit();

    } else if (itemData.mimeData->hasImage()) {
        widget->setImage(itemData.image);

        QBuffer buffer(&md5_data);
        buffer.open(QIODevice::WriteOnly);
        itemData.image.save(&buffer, "png");
        buffer.close();
    } else if (itemData.mimeData->hasUrls()) {
        QList<QUrl> urls = itemData.mimeData->urls();
        foreach(QUrl url, urls) {
            md5_data += url.toEncoded();
        }
        widget->setUrls(urls);
    } else if (itemData.mimeData->hasText() && !itemData.mimeData->text().isEmpty()) {
        widget->setPlainText(itemData.mimeData->text().trimmed());
        md5_data = itemData.mimeData->text().toLocal8Bit();
    } else {
        /* No data, remove it */
        QListWidgetItem *tmp_item = this->__scroll_widget->item(0);
        this->__scroll_widget->removeItemWidget(tmp_item);
        delete tmp_item;
        return;
    }

    itemData.md5 = QCryptographicHash::hash(md5_data, QCryptographicHash::Md5);
    /* Remove dup item */
    for (int i = 1; i < this->__scroll_widget->count(); i++) {
        QListWidgetItem *tmp_item = this->__scroll_widget->item(i);
        ItemData tmp_itemData = tmp_item->data(Qt::UserRole).value<ItemData>();
        /* They have same md5, remove it */
        if (itemData.md5 == tmp_itemData.md5) {
            /* move icon from old data */
            itemData.icon = tmp_itemData.icon;
            this->__db.delelePasteItem(tmp_itemData.md5);
            this->__scroll_widget->removeItemWidget(tmp_item);
            delete tmp_item;
            continue;
        }
        /* remove the data if it's too old (than a week) */
        if (QDateTime::currentDateTime().toSecsSinceEpoch() - tmp_itemData.time.toSecsSinceEpoch() >= (60 * 60 * 24 * 7)) {
            this->__db.delelePasteItem(tmp_itemData.md5);
            this->__scroll_widget->removeItemWidget(tmp_item);
            delete tmp_item;
        }
    }

    itemData.time = QDateTime::currentDateTime();
    widget->setTime(itemData.time);

    if (itemData.icon.isNull()) {
        /* Find and set icon who triggers the clipboard */
        itemData.icon = this->getClipboardOwnerIcon();
    }
    widget->setIcon(itemData.icon);
    this->__scroll_widget->item(0)->setData(Qt::UserRole, QVariant::fromValue(itemData));
    this->__db.insertPasteItem(&itemData);
}

QPixmap MainWindow::getClipboardOwnerIcon(void)
{
    QPixmap pixmap;

#ifdef Q_OS_WIN
    HWND hwnd = GetClipboardOwner();
    /* Get icon from Window */
    HICON icon = reinterpret_cast<HICON>(::SendMessageW(hwnd, WM_GETICON, ICON_BIG, 0));
    if (!icon)
        /* Try get icon from window class */
        icon = reinterpret_cast<HICON>(::GetClassLongPtr(hwnd, GCLP_HICON));
    if (!icon) {
        /* Find process id and get the process path, Final extract icons from executable files */
        DWORD pid;
        ::GetWindowThreadProcessId(hwnd, &pid);
        HANDLE processHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        TCHAR filename[MAX_PATH];
        DWORD cbNeeded;
        HMODULE hMod;
        if (processHandle) {
            if(::EnumProcessModules(processHandle, &hMod, sizeof(hMod), &cbNeeded)) {
                GetModuleFileNameEx(processHandle, NULL, filename, MAX_PATH);
                SHFILEINFO info;
                ZeroMemory(&info, sizeof(SHFILEINFO));
                unsigned int flags = SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_ICONLOCATION |
                    SHGFI_OPENICON | SHGFI_USEFILEATTRIBUTES;
                const HRESULT hr = SHGetFileInfo(filename, 0, &info, sizeof(SHFILEINFO), flags);
                if (FAILED(hr)) {
                    pixmap = QtWin::fromHICON(::LoadIcon(0, IDI_APPLICATION));
                } else  {
                    pixmap = pixmapFromShellImageList(0x4, info);
                    if (pixmap.isNull())
                        pixmap = pixmapFromShellImageList(0x2, info);
                    if (pixmap.isNull())
                        pixmap = QtWin::fromHICON(info.hIcon);
                    if (pixmap.isNull())
                        pixmap = QtWin::fromHICON(::LoadIcon(0, IDI_APPLICATION));
                }
            }
            ::CloseHandle(processHandle);
        } else {
            /* Failed, use default windows icon */
            pixmap = QtWin::fromHICON(::LoadIcon(0, IDI_APPLICATION));
        }
    } else {
        pixmap = QtWin::fromHICON(icon);
    }
#endif

#ifdef Q_OS_LINUX
    int i = 0;
    Display *display = XOpenDisplay(NULL);
    Atom clipboard_atom = XInternAtom(display, "CLIPBOARD", False);
    /* Search from [-100, 100] */
    Window clipboard_owner_win = XGetSelectionOwner(display, clipboard_atom) - 100;

    unsigned long nitems, bytesafter;
    unsigned char *ret;
    int format;
    Atom type;
    Atom wm_icon_atom = XInternAtom(display, "_NET_WM_ICON", True);
again:
    /* Get the width of the icon */
    XGetWindowProperty(display,
               clipboard_owner_win,
               wm_icon_atom,
               0, 1, 0,
               XA_CARDINAL,
               &type,
               &format,
               &nitems,
               &bytesafter,
               &ret);
    if (!ret) {
        /* FIXME: In fact, Get clipboard window id from XLIB is not the
         * actual window id, but it is strange that his actual ID is
         * near this, between -100 and +100.
         *
         * I didn't find out what happened, but he seems to be working.
         * if anyone finds a good way, please let me know.
         */
        clipboard_owner_win++;
        if (i++ > 200) {
            XCloseDisplay(display);
            qDebug() << "Not found icon, Use default Linux logo";
            pixmap.convertFromImage(QImage(":/resources/ubuntu.png"));
            return pixmap;
        }

        goto again;
    }

    int width = *(int *)ret;
    XFree(ret);

    /* Get the height of the Icon */
    XGetWindowProperty(display,
               clipboard_owner_win,
               wm_icon_atom,
               1, 1, 0,
               XA_CARDINAL,
               &type,
               &format,
               &nitems,
               &bytesafter,
               &ret);
    if (!ret) {
        qDebug() << "No X11 Icon height Found.";
        return pixmap;
    }

    int height = *(int *)ret;
    XFree(ret);

    /* Get data from Icon */
    int size = width * height;
    XGetWindowProperty(display,
               clipboard_owner_win,
               wm_icon_atom,
               2, size, 0,
               XA_CARDINAL,
               &type,
               &format,
               &nitems,
               &bytesafter,
               &ret);
    if (!ret) {
        qDebug() << "No X11 Icon Data Found.";
        return pixmap;
    }

    unsigned long *imgArr = (unsigned long*)(ret);
    std::vector<uint32_t> imgARGB32(size);
    for(int i=0; i<size; ++i)
        imgARGB32[i] = (uint32_t)(imgArr[i]);

    QImage *image = new QImage((uchar*)imgARGB32.data(), width, height, QImage::Format_ARGB32);
    pixmap.convertFromImage(*image);

    XFree(ret);
    delete image;
    XCloseDisplay(display);
#endif

    return pixmap;
}

void MainWindow::enabledGlassEffect(void)
{
#ifdef Q_OS_WIN
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0)) {
        HWND hWnd = HWND(this->winId());
        HMODULE hUser = GetModuleHandle(L"user32.dll");
        if (hUser) {
            pfnSetWindowCompositionAttribute setWindowCompositionAttribute =
                    (pfnSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");
            if (setWindowCompositionAttribute) {
                ACCENT_POLICY accent = { ACCENT_ENABLE_BLURBEHIND, 0, 0, 0 };
                WINDOWCOMPOSITIONATTRIBDATA data;
                data.Attrib = WCA_ACCENT_POLICY;
                data.pvData = &accent;
                data.cbData = sizeof(accent);
                setWindowCompositionAttribute(hWnd, &data);
            }
        }
    }
#endif

#ifdef Q_OS_LINUX
    KWindowEffects::enableBlurBehind(this->winId(), true);
#endif
}
