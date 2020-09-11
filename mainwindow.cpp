#include "mainwindow.h"

#include <QApplication>
#include <QScreen>
#include <QLabel>
#include <QSizePolicy>
#include <QScroller>
#include <QFile>
#include <QMessageBox>
#include <QMimeData>
#include <QImage>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QUrl>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <psapi.h>
#include <QOperatingSystemVersion>
#include <QtWinExtras/QtWinExtras>
#include <QtWinExtras/QtWin>
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
	  __shortcut(new QxtGlobalShortcut(this)),
	  __hide_state(true),
	  __clipboard(QApplication::clipboard()),
	  __is_me_trigger(false)
{
	QRect rect = QApplication::primaryScreen()->geometry();

	this->setGeometry(0, rect.height()*0.6, rect.width(), rect.height()*0.4);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
			     Qt::BypassWindowManagerHint | Qt::SplashScreen);
	this->setFocusPolicy(Qt::NoFocus);
	this->__main_frame->setGeometry(this->geometry());
	this->__main_frame->setObjectName(QString("MainFrame"));
	MainWindow::loadStyleSheet(this, ":/stylesheet.qss");
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
		QTimer::singleShot(50, this, SLOT(clipboard_later()));
	});
	QObject::connect(this->__hide_animation, &QPropertyAnimation::finished, [this](void) {
		if (this->__hide_animation->direction() == QAbstractAnimation::Forward) {
			/* Hidden stage */
			this->hide();
		} else {
			/* Show stage */
			this->activateWindow();
		}
	});
	this->__hide_animation->setDuration(200);
	this->__hide_animation->setStartValue(this->pos());
	this->__hide_animation->setEndValue(QPoint(0, QApplication::primaryScreen()->geometry().height()));
	this->__hide_animation->setEasingCurve(QEasingCurve::OutQuad);

#ifdef Q_OS_WIN
	QKeySequence key = QKeySequence("Ctrl+Shift+v");
#endif
#ifdef Q_OS_LINUX
	QKeySequence key = QKeySequence("Ctrl+Alt+v");
#endif
	this->__shortcut->setShortcut(key);
	QObject::connect(this->__shortcut, &QxtGlobalShortcut::activated, [this](void) {
		if (!this->isVisible())
			this->show_window();
	});

	this->initUI();

	/* Let show it, Just for test, Delete in the future */
	emit this->__shortcut->activated();
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
		widget->setFocus();
		widget->activateWindow();
		widget->show();
	}

	QWidget::showEvent(event);
}

void MainWindow::show_window(void)
{
	this->__hide_animation->setDirection(QAbstractAnimation::Backward);
	this->__hide_animation->start();
	this->__hide_state = false;
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
	auto *label = new QLabel(QString("HelloWord"));
	label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

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

	this->__hlayout = new QHBoxLayout();
	this->__hlayout->addStretch();
	this->__hlayout->addWidget(label);
	this->__hlayout->addStretch();

	this->__vlayout = new QVBoxLayout();
	this->__vlayout->addLayout(this->__hlayout);
	this->__vlayout->addWidget(this->__scroll_widget);

	this->__main_frame->setLayout(this->__vlayout);
	/* need this for resize this->__scroll_widget size */
	this->__main_frame->show();
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
		QMessageBox::information(nullptr, "tip", "cannot find qss file");
	}
}

/* Insert a PasteItem into listwidget */
PasteItem *MainWindow::insertItemWidget(QListWidgetItem **item)
{
	(*item) = new QListWidgetItem();
	auto *widget = new PasteItem(nullptr, *item);
	QObject::connect(widget, &PasteItem::hideWindow, [this](void) {
		this->__is_me_trigger = true;
		this->hide_window();
	});

	QRect rect = QApplication::primaryScreen()->geometry();
	/* resize item, It's use for pasteitem frame */
	(*item)->setSizeHint(QSize(rect.width()/6, 1));

	this->__scroll_widget->insertItem(0, *item);
	this->__scroll_widget->setCurrentRow(0);
	this->__scroll_widget->setItemWidget(*item, widget);
	this->resetItemTabOrder();

	return widget;
}

void MainWindow::resetItemTabOrder(void)
{
	int count = this->__scroll_widget->count();

	for (int i = 1; i < count; i++) {
		QWidget *first = this->__scroll_widget->itemWidget(this->__scroll_widget->item(i));
		QWidget *second = this->__scroll_widget->itemWidget(this->__scroll_widget->item(i+1));
		QWidget::setTabOrder(first, second);
	}
}

bool MainWindow::isMeTrigger(void)
{
	bool ret = false;

	/* Clipboard copy from myself? Yes is 'true', No is 'false' */
	if (this->__is_me_trigger)
		ret = true;

	/* Reset */
	this->__is_me_trigger = false;
	return ret;
}

void MainWindow::clipboard_later(void)
{
	const QMimeData *mime_data = this->__clipboard->mimeData();
	PasteItem *widget = nullptr;
	QListWidgetItem *item;

	if (isMeTrigger())
		return;

	widget = this->insertItemWidget(&item);
	ItemData itemData;

	if (mime_data->hasHtml()) {
		widget->setPlainText(mime_data->text().trimmed());
		itemData.type = ItemData::HTML;
		itemData.html = mime_data->html();
		itemData.text = mime_data->text();
	} else if (mime_data->hasImage()) {
		QImage image = qvariant_cast<QImage>(mime_data->imageData());
		widget->setImage(image);
		itemData.type = ItemData::IMAGE;
		itemData.image = image;
	} else if (mime_data->hasUrls()) {
		QList<QUrl> urls = mime_data->urls();
		QString s;
		foreach(QUrl url, urls) {
			s += url.toLocalFile() + "\n";
		}
		widget->setPlainText(s.trimmed());
		itemData.type = ItemData::URLS;
		itemData.urls = urls;
	} else if (mime_data->hasText()) {
		widget->setPlainText(mime_data->text().trimmed());
		itemData.type = ItemData::TEXT;
		itemData.text = mime_data->text();
	} else {
		/* No data, remove it */
		this->__scroll_widget->removeItemWidget(item);
	}

	/* Find and set icon who triggers the clipboard */
	widget->setIcon(this->getClipboardOwnerIcon());
	item->setData(Qt::UserRole, QVariant::fromValue(itemData));
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
				QFileInfo fileInfo(QString::fromWCharArray(filename));
				QFileIconProvider icon;
				pixmap = icon.icon(fileInfo).pixmap(32, 32);
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
		 * actual window id, but it is strange that But his actual ID is
		 * near this, between -100 and +100.
		 *
		 * I didn't find out what happened, but he seems to be working.
		 * if anyone finds a good way, please let me know.
		 */
		clipboard_owner_win++;
		if (i++ > 200) {
			XCloseDisplay(display);
			qDebug() << "Not found icon, Use default Linux logo";
			pixmap.convertFromImage(QImage(":/ubuntu.png"));
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
