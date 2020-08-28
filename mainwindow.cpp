#include "mainwindow.h"

#include <QApplication>
#include <QScreen>
#include <QLabel>
#include <QSizePolicy>
#include <QScroller>
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
	this->__main_frame->setGeometry(this->geometry());
	this->__main_frame->setStyleSheet("background-color: rgb(232, 232, 232);"
					  "border-top: 1px solid rgb(202, 202, 202);"
					  "border-top-right-radius: 3px;"
					  "border-top-left-radius: 3px;");
	this->setCentralWidget(this->__main_frame);
	this->setContentsMargins(0, 10, 0, 0);
	this->setAttribute(Qt::WA_TranslucentBackground, true);
	this->setFocusPolicy(Qt::StrongFocus);

	this->__main_frame_shadow->setOffset(0, 0);
	this->__main_frame_shadow->setColor(Qt::lightGray);
	this->__main_frame_shadow->setBlurRadius(10);
	this->__main_frame->setGraphicsEffect(this->__main_frame_shadow);

	QObject::connect(this->__hide_animation, &QPropertyAnimation::finished, [this](void) {
		if (this->__hide_animation->direction() == QAbstractAnimation::Forward) {
			/* Hidden stage */
			this->hide();
		} else {
			/* Show stage */
			this->activateWindow();
			this->setFocus();
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
		if (!this->isVisible()) {
			this->__hide_animation->setDirection(QAbstractAnimation::Backward);
			this->__hide_animation->start();
			this->show();
		}
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
		if (QApplication::activeWindow() != this) {
			this->__hide_animation->setDirection(QAbstractAnimation::Forward);
			this->__hide_animation->start();
		}
	}

	return QMainWindow::event(e);
}

void MainWindow::initUI(void)
{
	QLabel *label = new QLabel(QString("HelloWord"));
	label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	this->__scroll_widget = new QListWidget();
	this->__scroll_widget->setHorizontalScrollMode(QListWidget::ScrollPerPixel);
	this->__scroll_widget->setFlow(QListView::LeftToRight);
	this->__scroll_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->__scroll_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QScroller::grabGesture(this->__scroll_widget, QScroller::LeftMouseButtonGesture);

	this->__hlayout = new QHBoxLayout();
	this->__vlayout = new QVBoxLayout();

	this->__hlayout->addStretch();
	this->__hlayout->addWidget(label);
	this->__hlayout->addStretch();

	for (int i = 0; i < 100; i++) {
		QListWidgetItem *item = new QListWidgetItem("123123");
		QLabel *x_label = new QLabel("2222222");
		this->__scroll_widget->setItemWidget(item, x_label);
		this->__scroll_widget->addItem(item);
	}

	this->__vlayout->addLayout(this->__hlayout);
	this->__vlayout->addWidget(this->__scroll_widget);
	this->__main_frame->setLayout(this->__vlayout);
}
