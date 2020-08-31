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
#ifdef Q_OS_WIN
#include <windows.h>
#include <winuser.h>
#include <QOperatingSystemVersion>
#endif
#include <QDebug>

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
	  __main_frame(new QFrame(this)),
	  __main_frame_shadow(new QGraphicsDropShadowEffect(this)),
	  __hide_animation(new QPropertyAnimation(this, "pos")),
	  __shortcut(new QxtGlobalShortcut(this)),
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
	MainWindow::loadStyleSheet(this->__main_frame, ":/stylesheet.qss");
	this->setCentralWidget(this->__main_frame);
#if !defined Q_OS_LINUX && !defined Q_OS_WIN
	this->setContentsMargins(0, 10, 0, 0);
#endif
	this->setAttribute(Qt::WA_TranslucentBackground, true);
#ifdef Q_OS_WIN
	if (QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0)) {
		HWND hWnd = HWND(winId());
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
#ifndef Q_OS_WINDOWS
	this->__hide_animation->setEasingCurve(QEasingCurve::OutQuad);
#endif

	this->__shortcut->setShortcut(QKeySequence("Ctrl+Shift+v"));
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
	this->__vlayout = new QVBoxLayout();

	this->__hlayout->addStretch();
	this->__hlayout->addWidget(label);
	this->__hlayout->addStretch();

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

PasteItem *MainWindow::insertItemWidget(void)
{
	auto *widget = new PasteItem();

	QObject::connect(widget, SIGNAL(hideWindow()), this, SLOT(hide_window()));

	QRect rect = QApplication::primaryScreen()->geometry();
	auto *item = new QListWidgetItem();
	item->setSizeHint(QSize(rect.width()/6, this->__scroll_widget->height()));

	this->__scroll_widget->addItem(item);
	widget->resize(item->sizeHint());

	this->__scroll_widget->setItemWidget(item, widget);

	return widget;
}

void MainWindow::clipboard_later(void)
{
	const QMimeData *mime_data = this->__clipboard->mimeData();

	if (mime_data->hasImage()) {
		auto *widget = this->insertItemWidget();
		QImage image = qvariant_cast<QImage>(mime_data->imageData());
		widget->setImage(image);
	} else if (mime_data->hasText()) {
		auto widget = this->insertItemWidget();
		widget->setPlainText(mime_data->text().trimmed());
	}
}
