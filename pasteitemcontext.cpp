#include "pasteitemcontext.h"

#include <QResizeEvent>
#include <QDebug>

PixmapFrame::PixmapFrame(QWidget *parent) : QLabel(parent)
{
	this->setObjectName("ContextPixmapFrame");
}

void PixmapFrame::resizeEvent(QResizeEvent *event)
{
	QSize size = event->size();

	if (!m_pixmap.isNull()) {
		this->setPixmap(m_pixmap.scaled(size,
						Qt::KeepAspectRatio,
						Qt::SmoothTransformation));
	}

	QLabel::resizeEvent(event);
}

StackedWidget::StackedWidget(QWidget *parent) : QStackedWidget(parent),
	m_pixmap_frame(new PixmapFrame(this)),
	m_text_frame(new QLabel(this))
{
	m_text_frame->setObjectName("ContextTextFrame");
	m_text_frame->setWordWrap(true);
	m_text_frame->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	this->setObjectName("Context");

	this->addWidget(m_pixmap_frame);
	this->addWidget(m_text_frame);
}

void StackedWidget::setPixmap(QPixmap &pixmap)
{
	m_pixmap_frame->setStorePixmap(pixmap);
	this->setCurrentIndex(0);
}

const QPixmap *StackedWidget::pixmap(void)
{
	return m_pixmap_frame->pixmap();
}

void StackedWidget::setText(QString &s)
{
	m_text_frame->setText(s);
	this->setCurrentIndex(1);
}

QString StackedWidget::text(void)
{
	return m_text_frame->text();
}
