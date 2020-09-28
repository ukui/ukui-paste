#include <QEvent>
#include <QKeyEvent>

#include "mainframe.h"

MainFrame::MainFrame(QWidget *parent) : QWidget(parent)
{
	this->setObjectName("MainFrame");
	this->setAttribute(Qt::WA_StyledBackground);
}

bool MainFrame::event(QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *ke = static_cast<QKeyEvent *>(event);

		switch (ke->key()) {
		case Qt::Key_Tab:
			emit this->moveFocusPrevNext(false);
			return true;
		case Qt::Key_Backtab:
			emit this->moveFocusPrevNext(true);
			return true;
		}
	}

	return QWidget::event(event);
}
