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
#include <QTextEdit>

#include "mainwindow.h"
#include <KF5/KWindowSystem/KWindowEffects>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

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
    initPanelDbusGsetting();
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

    QObject::connect(this->__shortcut, &DoubleCtrlShortcut::activated, [this](void) {
        /*整体窗口位置*/
        window_xy();

        if (!this->isVisible())
            this->show_window();
        else
            this->hide_window();
    });

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                     Qt::BypassWindowManagerHint | Qt::SplashScreen);
    this->setFocusPolicy(Qt::NoFocus);
    this->setCentralWidget(this->__main_frame);
    this->setAttribute(Qt::WA_TranslucentBackground, false);
    this->enabledGlassEffect();

    this->__main_frame->setGeometry(this->geometry());
    this->__main_frame->setFocusPolicy(Qt::ClickFocus);
    QObject::connect(this->__main_frame, SIGNAL(moveFocusPrevNext(bool)), this, SLOT(move_to_prev_next_focus_widget(bool)));

    QShortcut *shortcut_search = new QShortcut(this);
    shortcut_search->setKey(QKeySequence("Ctrl+f"));
    QObject::connect(shortcut_search, &QShortcut::activated, [this](void) {
        LineEdit *lineedit = this->__searchbar->findChild<LineEdit *>("", Qt::FindDirectChildrenOnly);
        lineedit->setFocus();
    });

    this->initUI();
    return;
}

void MainWindow::initPanelDbusGsetting(){
    /* 链接任务栏Dbus接口，获取任务栏高度和位置 */
    m_pServiceInterface = new QDBusInterface(PANEL_DBUS_SERVICE, PANEL_DBUS_PATH, PANEL_DBUS_INTERFACE, QDBusConnection::sessionBus());
    m_pServiceInterface->setTimeout(2147483647);

    /* 链接任务栏dgsetting接口 */
    if(QGSettings::isSchemaInstalled(UKUI_PANEL_SETTING))
        m_pPanelSetting = new QGSettings(UKUI_PANEL_SETTING);
}

/*获取桌面是否有改变*/
void MainWindow::desktoparea_change(){
    QDesktopWidget* desktopWidget = QApplication::desktop();
    connect(desktopWidget, &QDesktopWidget::resized, this, &MainWindow::window_xy);
}

/*整体窗口位置*/
void MainWindow::window_xy(){
    desktop_width();
    desktop_height();
    switch(getPanelSite()){
        case 0:
            this->setGeometry(8, screenHeight*0.68, screenWidth-16, screenHeight*0.32-connectTaskBarDbus()-8);
            break;
        case 1:
            this->setGeometry(8, screenHeight*0.68+connectTaskBarDbus(), screenWidth-16, screenHeight*0.32-connectTaskBarDbus()-8);
            break;
        case 2:
            this->setGeometry(8+connectTaskBarDbus(), screenHeight*0.68, screenWidth-16-connectTaskBarDbus(), screenHeight*0.32-8);
            break;
        case 3:
            this->setGeometry(8, screenHeight*0.68, screenWidth-16-connectTaskBarDbus(), screenHeight*0.32-8);
            break;
        default:
            break;
    }
}

/*获取主桌面分辨率大小*/
int MainWindow::desktop_width(){
    QRect screenRect1 = desktop->screenGeometry();
    screenWidth = screenRect1.width();
    return screenWidth;
}
int MainWindow::desktop_height(){
    QRect screenRect2 = desktop->screenGeometry();
    screenHeight = screenRect2.height();
    return screenHeight;
}

/*获取任务栏的大小*/
int MainWindow::connectTaskBarDbus(){
    int panelHeight = 46;
    if (m_pPanelSetting != nullptr) {
        QStringList keys = m_pPanelSetting->keys();
        if (keys.contains("panelsize")) {
            panelHeight = m_pPanelSetting->get("panelsize").toInt();
        }
    } else {
        QDBusMessage msg = m_pServiceInterface->call("GetPanelSize", QVariant("Hight"));
        panelHeight = msg.arguments().at(0).toInt();
        return panelHeight;
    }
    return panelHeight;
}

/*获取任务栏位置*/
int MainWindow::getPanelSite(){
    int panelPosition = 0;
    if (m_pPanelSetting != nullptr) {
        QStringList keys = m_pPanelSetting->keys();
        if (keys.contains("panelposition")) {
            panelPosition = m_pPanelSetting->get("panelposition").toInt();
        }
    } else {
        QDBusMessage msg = m_pServiceInterface->call("GetPanelPosition", QVariant("Site"));
        panelPosition = msg.arguments().at(0).toInt();
    }
    return panelPosition;
}


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
    /*搜索部分*/
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

    /*分类部分*/
    this->__classify1 = new PushButton(this->__main_frame);
    this->__classify1->setText("fuwenben");
    this->__classify1->setFixedSize(80, 30);
    this->__classify1->setFlat(true);
    this->__classify2 = new PushButton(this->__main_frame);
    this->__classify2->setText("pics");
    this->__classify2->setFixedSize(80, 30);
    this->__classify2->setFlat(true);
    this->__classify3 = new PushButton(this->__main_frame);
    this->__classify3->setText("wenjian");
    this->__classify3->setFixedSize(80, 30);
    this->__classify3->setFlat(true);
    this->__classify4 = new PushButton(this->__main_frame);
    this->__classify4->setText("wenben");
    this->__classify4->setFixedSize(80, 30);
    this->__classify4->setFlat(true);
    this->__classify5 = new PushButton(this->__main_frame);
    this->__classify5->setText("all");
    this->__classify5->setFixedSize(80, 30);
    this->__classify5->setFlat(true);

    QObject::connect(this->__classify1, &PushButton::clicked, [this](void) {
        this->reloadData(1);
    });
    QObject::connect(this->__classify2, &PushButton::clicked, [this](void) {
        this->reloadData(2);
    });
    QObject::connect(this->__classify3, &PushButton::clicked, [this](void) {
        this->reloadData(3);
    });
    QObject::connect(this->__classify4, &PushButton::clicked, [this](void) {
        this->reloadData(4);
    });
    QObject::connect(this->__classify5, &PushButton::clicked, [this](void) {
        this->reloadData(0);
    });


    /*展示部分*/
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

    /*整体布局*/
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(this->__searchbar);
    hlayout->addWidget(this->__classify5);
    hlayout->addWidget(this->__classify1);
    hlayout->addWidget(this->__classify2);
    hlayout->addWidget(this->__classify3);
    hlayout->addWidget(this->__classify4);
    hlayout->addStretch();

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addLayout(hlayout);
    vlayout->addWidget(this->__scroll_widget);

    this->__main_frame->setLayout(vlayout);
    /* need this for resize this->__scroll_widget size */
    this->__main_frame->show();

    /* load data from database */
    this->reloadData(0);
}

/*处理复制粘贴的展示数据*/
void MainWindow::reloadData(int format){
    /*第一次打开时创造数据表*/
    if (!this->__db.isTableExist()) {
        this->__db.createTable();
        return;
    }
    int y =this->__scroll_widget->count();
    for (int x=0 ;x<=y;x++){
        delete this->__scroll_widget->takeItem(0);
        int y = y+1;
    }

    for (auto itemData : this->__db.loadData()) {
        /* 时间过长删除 */
        if (QDateTime::currentDateTime().toSecsSinceEpoch() - itemData->time.toSecsSinceEpoch() > (60 * 60 * 24 * 7)) {
            this->__db.delelePasteItem(itemData->md5);
            continue;
        }

        if(format == 1){
            PasteItem *widgetRichText = this->insertItemWidgetRichText();
            if (itemData->mimeData->hasHtml() && !itemData->mimeData->text().isEmpty()) {
                widgetRichText->setRichText(itemData->mimeData->html(), itemData->mimeData->text());
            } else {
                /* No data, remove it */
                delete this->__scroll_widget->takeItem(0);
                continue;
            }
            widgetRichText->setTime(itemData->time);
            widgetRichText->setIcon(itemData->icon);

        } else if (format == 2){

            PasteItem *widgetImage = this->insertItemWidgetImage();
            if (itemData->mimeData->hasImage()) {
                widgetImage->setImage(itemData->image);
            } else {
                /* No data, remove it */
                delete this->__scroll_widget->takeItem(0);
                continue;
            }
            widgetImage->setTime(itemData->time);
            widgetImage->setIcon(itemData->icon);

        } else if(format == 3){

            PasteItem *widgetUrls = this->insertItemWidgetUrls();
            QList<QUrl> urls = itemData->mimeData->urls();
            if (itemData->mimeData->hasUrls()) {
                if (!widgetUrls->setUrls(urls)) {
                    this->__db.delelePasteItem(itemData->md5);
                    delete this->__scroll_widget->takeItem(0);
                    continue;
                }
            } else {
                /* No data, remove it */
                delete this->__scroll_widget->takeItem(0);
                continue;
            }
            widgetUrls->setTime(itemData->time);
            widgetUrls->setIcon(itemData->icon);

        } else if (format == 4){

            PasteItem *widgetPlainText = this->insertItemWidgetPlainText();
            if (itemData->mimeData->hasText() && !itemData->mimeData->text().isEmpty()) {
                widgetPlainText->setPlainText(itemData->mimeData->text().trimmed());
            } else {
                /* No data, remove it */
                delete this->__scroll_widget->takeItem(0);
                continue;
            }
            widgetPlainText->setTime(itemData->time);
            widgetPlainText->setIcon(itemData->icon);

        } else {

            PasteItem *widget = this->insertItemWidget();
            /*复制网页内容 空内容时 链接 图片 文本 以及文件地址*/
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

        }

        this->__scroll_widget->item(0)->setData(Qt::UserRole, QVariant::fromValue(*itemData));
    }
}

/* Insert a PasteItem into listwidget */
/* 插入一个可以粘贴的内容 */
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

PasteItem *MainWindow::insertItemWidgetRichText(void)
{
    QListWidgetItem *item = new QListWidgetItem;
    auto *widgetRichText = new PasteItem(nullptr, item);

    QObject::connect(widgetRichText, &PasteItem::hideWindow, [this](void) {
        this->hide_window();
    });

    QRect rect = QApplication::primaryScreen()->geometry();
    /* resize item, It's use for pasteitem frame */
    item->setSizeHint(QSize(rect.width()/6, 1));

    this->__scroll_widget->insertItem(0, item);
    this->__scroll_widget->setCurrentRow(0);
    this->__scroll_widget->setItemWidget(item, widgetRichText);
    this->resetItemTabOrder();

    return widgetRichText;
}

PasteItem *MainWindow::insertItemWidgetImage(void)
{
    QListWidgetItem *item = new QListWidgetItem;
    auto *widgetImage = new PasteItem(nullptr, item);

    QObject::connect(widgetImage, &PasteItem::hideWindow, [this](void) {
        this->hide_window();
    });

    QRect rect = QApplication::primaryScreen()->geometry();
    /* resize item, It's use for pasteitem frame */
    item->setSizeHint(QSize(rect.width()/6, 1));

    this->__scroll_widget->insertItem(0, item);
    this->__scroll_widget->setCurrentRow(0);
    this->__scroll_widget->setItemWidget(item, widgetImage);
    this->resetItemTabOrder();

    return widgetImage;
}

PasteItem *MainWindow::insertItemWidgetUrls(void)
{
    QListWidgetItem *item = new QListWidgetItem;
    auto *widgetUrls = new PasteItem(nullptr, item);

    QObject::connect(widgetUrls, &PasteItem::hideWindow, [this](void) {
        this->hide_window();
    });

    QRect rect = QApplication::primaryScreen()->geometry();
    /* resize item, It's use for pasteitem frame */
    item->setSizeHint(QSize(rect.width()/6, 1));

    this->__scroll_widget->insertItem(0, item);
    this->__scroll_widget->setCurrentRow(0);
    this->__scroll_widget->setItemWidget(item, widgetUrls);
    this->resetItemTabOrder();

    return widgetUrls;
}

PasteItem *MainWindow::insertItemWidgetPlainText(void)
{
    QListWidgetItem *item = new QListWidgetItem;
    auto *widgetPlainText = new PasteItem(nullptr, item);

    QObject::connect(widgetPlainText, &PasteItem::hideWindow, [this](void) {
        this->hide_window();
    });

    QRect rect = QApplication::primaryScreen()->geometry();
    /* resize item, It's use for pasteitem frame */
    item->setSizeHint(QSize(rect.width()/6, 1));

    this->__scroll_widget->insertItem(0, item);
    this->__scroll_widget->setCurrentRow(0);
    this->__scroll_widget->setItemWidget(item, widgetPlainText);
    this->resetItemTabOrder();

    return widgetPlainText;
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
