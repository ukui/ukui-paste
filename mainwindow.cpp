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
	  __main_frame(new QWidget(this)),
	  __main_frame_shadow(new QGraphicsDropShadowEffect(this)),
	  __hide_animation(new QPropertyAnimation(this, "pos")),
	  __shortcut(new DoubleCtrlShortcut(this)),
	  __hide_state(true),
	  __clipboard(QApplication::clipboard())
{
	QRect rect = QApplication::primaryScreen()->geometry();

	this->setGeometry(0, rect.height()*0.6, rect.width(), rect.height()*0.4);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
			     Qt::BypassWindowManagerHint | Qt::SplashScreen);
	this->setFocusPolicy(Qt::NoFocus);
	this->__main_frame->setGeometry(this->geometry());
	this->__main_frame->setObjectName(QString("MainFrame"));
	MainWindow::loadStyleSheet(this, ":/resources/stylesheet.qss");
	this->setCentralWidget(this->__main_frame);
#if !defined Q_OS_LINUX && !defined Q_OS_WIN
	this->setContentsMargins(0, 10, 0, 0);
#endif
	this->setAttribute(Qt::WA_TranslucentBackground, true);
	this->enabledGlassEffect();

	this->__main_frame_shadow->setOffset(0, 0);
	this->__main_frame_shadow->setColor(Qt::lightGray);
	this->__main_frame_shadow->setBlurRadius(10);
	this->__main_frame->setGraphicsEffect(this->__main_frame_shadow);
	this->__main_frame->setFocusPolicy(Qt::NoFocus);

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
	auto lists = this->__scroll_widget->selectedItems();
	if(lists.length() > 0) {
		auto *widget = this->__scroll_widget->itemWidget(lists.value(0));
		/* Let the selected item has focus */
		widget->setFocus();
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

void MainWindow::initUI(void)
{
	this->__searchbar = new SearchBar(this->__main_frame, 2*this->width()/5, 40);
	QObject::connect(this->__searchbar, &SearchBar::hideWindow, [this](void) {
		this->hide_window();
	});
	QObject::connect(this->__searchbar, &SearchBar::textChanged, [this](const QString &text) {
		for (int i = 0; i < this->__scroll_widget->count(); i++) {
			QListWidgetItem *item = this->__scroll_widget->item(i);
			PasteItem *widget = reinterpret_cast<PasteItem *>(this->__scroll_widget->itemWidget(item));
			if (!widget->text().contains(text)) {
				item->setHidden(true);
			} else {
				item->setHidden(false);
			}
		}
	});

	this->__menu_button = new PushButton(this->__main_frame);
	this->__menu_button->setPixmap(QPixmap(":/resources/points.png"));
	this->__menu_button->setFixedSize(30, 30);
	this->__menu_button->setFlat(true);
	QObject::connect(this->__menu_button, &PushButton::clicked, [this](void) {
		QMenu *menu = new QMenu();
		QAction *about_me = new QAction(QObject::tr("About me"), this);
		QObject::connect(about_me, &QAction::triggered, [this](void) {
			QMessageBox::about(this, QObject::tr("About me"), "Powered by Jackie Liu <liuyun01@kylinos.cn>");
		});

		menu->addAction(about_me);

		menu->exec(this->cursor().pos());
		QList<QAction *> actions = menu->actions();
		for(auto action : actions) {
			delete action;
		}
		delete menu;
	});

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

	QHBoxLayout *hlayout = new QHBoxLayout();
	hlayout->addStretch();
	hlayout->addWidget(this->__searchbar);
	hlayout->addStretch();
	hlayout->addWidget(this->__menu_button);

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
			widget->setUrls(urls);
		} else if (itemData->mimeData->hasText() && !itemData->mimeData->text().isEmpty()) {
			widget->setPlainText(itemData->mimeData->text().trimmed());
		} else {
			/* No data, remove it */
			this->__scroll_widget->takeItem(0);
			continue;
		}
		widget->setTime(itemData->time);
		widget->setIcon(itemData->icon);

		this->__scroll_widget->item(0)->setData(Qt::UserRole, QVariant::fromValue(*itemData));
	}
}

void MainWindow::loadStyleSheet(QWidget *w, const QString &styleSheetFile)
{
	QFile file(styleSheetFile);
	file.open(QFile::ReadOnly);
	if (file.isOpen()) {
		QString styleSheet = w->styleSheet();
		styleSheet += QLatin1String(file.readAll());
		w->setStyleSheet(styleSheet);
		file.close();
	} else {
		QMessageBox::information(nullptr, QObject::tr("tips"), QObject::tr("cannot find qss file"));
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
