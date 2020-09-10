#include "pasteitem.h"

#include <QClipboard>
#include <QApplication>
#include <QVBoxLayout>
#include <QDebug>

PasteItem::PasteItem(QWidget *parent) : QWidget(parent),
	m_frame(new QWidget(this)),
	m_frame_effect(new QGraphicsDropShadowEffect(this)),
	m_barnner(new Barnner(this->m_frame)),
	m_context(new StackedWidget(this->m_frame))
{
	this->setFocusPolicy(Qt::StrongFocus);
	this->setAttribute(Qt::WA_TranslucentBackground);

	m_frame_effect->setOffset(0, 0);
	m_frame_effect->setColor(Qt::gray);
	m_frame_effect->setBlurRadius(8);
	m_frame->setGraphicsEffect(m_frame_effect);
	m_frame->setObjectName("PasteItemFrame");

	QVBoxLayout *vboxlayout = new QVBoxLayout();
	vboxlayout->addWidget(m_barnner);
	vboxlayout->addWidget(m_context);
	vboxlayout->setSpacing(0);
	vboxlayout->setContentsMargins(5, 5, 5, 5);

	m_frame->setLayout(vboxlayout);
	m_frame->show();
}

void PasteItem::setImage(QImage &image)
{
	QPixmap pixmap = QPixmap::fromImage(image);
	m_context->setPixmap(pixmap);
}

void PasteItem::setPlainText(QString s)
{
	m_context->setText(s);
}

void PasteItem::setIcon(QPixmap pixmap)
{
	m_barnner->setIcon(pixmap);
}

void PasteItem::resizeEvent(QResizeEvent *event)
{
	QSize size = event->size();
	m_frame->resize(size);
	m_barnner->setFixedHeight(size.height()/5);

	QWidget::resizeEvent(event);
}

void PasteItem::mouseDoubleClickEvent(QMouseEvent *event)
{
	this->copyData();

	QWidget::mouseDoubleClickEvent(event);
}

void PasteItem::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Return:
	case Qt::Key_Enter:
		this->copyData();
		break;
	case Qt::Key_Escape:
		emit this->hideWindow();
		break;
	}

	QWidget::keyPressEvent(event);
}

void PasteItem::copyData(void)
{
	QClipboard *clipboard = QApplication::clipboard();

	/* That is image, copy it */
	if (m_context->currentIndex() == StackedWidget::IMAGE)
		clipboard->setImage(m_context->pixmap()->toImage());
	/* That is text, copy it */
	if (m_context->currentIndex() == StackedWidget::TEXT)
		clipboard->setText(m_context->text());

	emit this->hideWindow();
}
