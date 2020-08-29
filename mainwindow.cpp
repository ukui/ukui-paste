#include "mainwindow.h"

#include <QApplication>
#include <QScreen>
#include <QLabel>
#include <QSizePolicy>
#include <QScroller>
#include <QFile>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	  __main_frame(new QFrame(this)),
	  __main_frame_shadow(new QGraphicsDropShadowEffect(this)),
	  __hide_animation(new QPropertyAnimation(this, "pos")),
	  __shortcut(new QxtGlobalShortcut(this))
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
	this->setContentsMargins(0, 10, 0, 0);
	this->setAttribute(Qt::WA_TranslucentBackground, true);

	this->__main_frame_shadow->setOffset(0, 0);
	this->__main_frame_shadow->setColor(Qt::lightGray);
	this->__main_frame_shadow->setBlurRadius(10);
	this->__main_frame->setGraphicsEffect(this->__main_frame_shadow);
	this->__main_frame->setFocusPolicy(Qt::NoFocus);

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
	this->show();
}

void MainWindow::hide_window(void)
{
	this->__hide_animation->setDirection(QAbstractAnimation::Forward);
	this->__hide_animation->start();
}

void MainWindow::initUI(void)
{
	QRect rect = QApplication::primaryScreen()->geometry();
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

	for (int i = 0; i < 30; i++) {
		auto *item = new QListWidgetItem();
		item->setSizeHint(QSize(rect.width()/6, this->__scroll_widget->height()));

		this->__scroll_widget->addItem(item);
		auto widget = this->createItemWidget();
		widget->resize(item->sizeHint());

		this->__scroll_widget->setItemWidget(item, widget);
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
		QMessageBox::information(nullptr, "tip", "cannot find qss file");
	}
}

QWidget *MainWindow::createItemWidget()
{
	auto *widget = new PasteItem(this);

	QObject::connect(widget, SIGNAL(click()), this, SLOT(hide_window()));
	widget->setStyleSheet("background-color: red; border-top-right-radius: 10px;"
			      "border-top-left-radius: 10px;");
	widget->show();
	return widget;
}
