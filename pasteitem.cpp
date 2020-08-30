#include "pasteitem.h"

#include <QDebug>

PasteItem::PasteItem(QWidget *parent) : QWidget(parent),
	m_frame(new QWidget(this)),
	m_frame_effect(new QGraphicsDropShadowEffect(this)),
	m_pixmap(nullptr),
	m_plaintext(nullptr),
	m_richtext(nullptr)
{
	this->setFocusPolicy(Qt::StrongFocus);
	this->setAttribute(Qt::WA_TranslucentBackground);
	m_frame->setStyleSheet("margin: 5px;"
			       "padding: 10px;"
			       "border: 1px solid rgb(202, 202, 202);"
			       "background-color: white;"
			       "border-top-right-radius: 10px;"
			       "border-top-left-radius: 10px;"
			       "color: #333;");
	m_frame_effect->setOffset(0, 0);
	m_frame_effect->setColor(Qt::lightGray);
	m_frame_effect->setBlurRadius(10);
	m_frame->setGraphicsEffect(m_frame_effect);
	m_frame->show();
}

void PasteItem::setImage(QImage &image)
{
	m_frame->setStyleSheet(m_frame->styleSheet() +
			       "background-image: url(:/background.jpg);"
			       "background-repeat: repeat-xy;"
			       "background-position: top left;"
			       "background-color: transparent;"
			       "padding: 0px;");
	m_frame->setAutoFillBackground(true);
	QPixmap pixmap = QPixmap::fromImage(image);
	m_pixmap = new PixmapFrame(this->m_frame);
	m_pixmap->setPixmap(pixmap);
	m_pixmap->setFocusPolicy(Qt::NoFocus);
	m_pixmap->show();
}

void PasteItem::setRichText(QString s)
{
	m_richtext = new QLabel(this->m_frame);
	m_richtext->setText(s);
	m_richtext->setFocusPolicy(Qt::NoFocus);
	m_richtext->setTextFormat(Qt::RichText);
	m_richtext->show();
}

void PasteItem::setPlainText(QString s)
{
	m_plaintext = new QLabel(this->m_frame);
	m_plaintext->setText(s);
	m_plaintext->setFocusPolicy(Qt::NoFocus);
	m_plaintext->setTextFormat(Qt::PlainText);
	m_plaintext->show();
}

void PasteItem::resizeEvent(QResizeEvent *event)
{
	QSize size = event->size();
	size.setHeight(size.height()-20);

	m_frame->resize(size);
	if (m_pixmap)
		m_pixmap->resize(size);
	if (m_richtext)
		m_richtext->resize(size);
	if (m_plaintext)
		m_plaintext->resize(size);

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
