#include "pasteitem.h"

#include <QDebug>

PasteItem::PasteItem(QWidget *parent) : QWidget(parent),
      m_frame(new QFrame(this))
{
	m_frame->setStyleSheet("background-color: green");
	this->setFocusPolicy(Qt::StrongFocus);
}

void PasteItem::resizeEvent(QResizeEvent *event)
{
	QSize size = event->size();
	size.setHeight(size.height()-30);
	m_frame->resize(size);

	QWidget::resizeEvent(event);
}

void PasteItem::mouseDoubleClickEvent(QMouseEvent *event)
{
	emit this->click();

	QWidget::mouseDoubleClickEvent(event);
}

void PasteItem::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Return:
	case Qt::Key_Enter:
		emit this->click();
	}

	QWidget::keyPressEvent(event);
}
