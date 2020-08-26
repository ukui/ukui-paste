#include "mainwindow.h"

#include <QApplication>
#include <QPropertyAnimation>
#include <QScreen>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	QRect rect = QApplication::primaryScreen()->geometry();

	this->setGeometry(0, rect.height()*0.6, rect.width(), rect.height()*0.4);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SplashScreen);
	this->setAttribute(Qt::WA_DeleteOnClose);
}

MainWindow::~MainWindow()
{
}

void MainWindow::focusOutEvent(QFocusEvent *)
{
	QPropertyAnimation *hidden_animotion = new QPropertyAnimation(this, "pos");
	QObject::connect(hidden_animotion, SIGNAL(finished()), this, SLOT(close()));

	hidden_animotion->setDuration(400);
	hidden_animotion->setStartValue(this->pos());
	hidden_animotion->setEndValue(QPoint(0, QApplication::primaryScreen()->geometry().height()));
	hidden_animotion->setEasingCurve(QEasingCurve::OutQuad);
	hidden_animotion->start(QAbstractAnimation::DeleteWhenStopped);
}
