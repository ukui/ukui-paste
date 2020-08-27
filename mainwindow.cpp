#include "mainwindow.h"

#include <QApplication>
#include <QScreen>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	  __hide_animation(new QPropertyAnimation(this, "pos"))
{
	QRect rect = QApplication::primaryScreen()->geometry();

	qDebug() << rect;

	this->setGeometry(0, rect.height()*0.6, rect.width(), rect.height()*0.4);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
#ifdef Q_OS_LINUX
			     Qt::X11BypassWindowManagerHint |
#endif
			     Qt::SplashScreen);

	QObject::connect(this->__hide_animation, SIGNAL(finished()), this, SLOT(hide()));
	this->__hide_animation->setDuration(350);
	this->__hide_animation->setStartValue(this->pos());
	this->__hide_animation->setEndValue(QPoint(0, QApplication::primaryScreen()->geometry().height()));
	this->__hide_animation->setEasingCurve(QEasingCurve::OutQuad);
}

MainWindow::~MainWindow()
{
	delete this->__hide_animation;
}

void MainWindow::focusOutEvent(QFocusEvent *)
{
	this->__hide_animation->start();
}
