#include "mainwindow.h"

#include <QApplication>
#include <QScreen>
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
					  "border-top: 1px solid rgb(202, 202, 202);");
	this->setCentralWidget(this->__main_frame);
	this->setContentsMargins(0, 10, 0, 0);
	this->setAttribute(Qt::WA_TranslucentBackground, true);
	this->setFocusPolicy(Qt::StrongFocus);

	this->__main_frame_shadow->setOffset(0, 0);
	this->__main_frame_shadow->setColor(Qt::lightGray);
	this->__main_frame_shadow->setBlurRadius(10);
	this->__main_frame->setGraphicsEffect(this->__main_frame_shadow);

	QObject::connect(this->__hide_animation, &QPropertyAnimation::finished, [this](void) {
		/* If in hidden stage, hide it */
		if (this->__hide_animation->direction() == QAbstractAnimation::Forward) {
			this->hide();
		} else {
			this->activateWindow();
			this->setFocus();
		}
	});
	this->__hide_animation->setDuration(350);
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

	/* Let show it */
	emit this->__shortcut->activated();
}

MainWindow::~MainWindow()
{
	delete this->__hide_animation;
}

void MainWindow::focusOutEvent(QFocusEvent *)
{
	this->__hide_animation->setDirection(QAbstractAnimation::Forward);
	this->__hide_animation->start();
}
