#include "pasteitemcontext.h"

#include <QResizeEvent>
#include <QDebug>

TextFrame::TextFrame(QWidget *parent) : QLabel(parent),
	m_mask_label(new QLabel(this))
{
	this->setObjectName("ContextTextFrame");
	this->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	this->setWordWrap(true);

	this->m_mask_label->setAlignment(Qt::AlignCenter);
	this->m_mask_label->setContentsMargins(0, 3, 0, 3);
}

TextFrame::~TextFrame()
{
	delete m_mask_label;
}

void TextFrame::setMaskFrameText(QString s)
{
	this->m_mask_label->setText(s);
	this->m_mask_label->show();
}

void TextFrame::resizeEvent(QResizeEvent *event)
{
	this->m_mask_label->setGeometry(0, this->height()-LABEL_HEIGHT,
					this->width(), LABEL_HEIGHT);

	QLabel::resizeEvent(event);
}

PixmapFrame::PixmapFrame(QWidget *parent) : TextFrame(parent)
{
	this->setObjectName("ContextPixmapFrame");
	this->setAlignment(Qt::AlignCenter);
}

void PixmapFrame::resizeEvent(QResizeEvent *event)
{
	if (!m_pixmap.isNull()) {
		this->setPixmap(m_pixmap.scaled(event->size(),
						Qt::KeepAspectRatio,
						Qt::SmoothTransformation));
	}

	TextFrame::resizeEvent(event);
}

StackedWidget::StackedWidget(QWidget *parent) : QStackedWidget(parent),
	m_pixmap_frame(new PixmapFrame(this)),
	m_text_frame(new TextFrame(this)),
	m_richtext_frame(new TextFrame(this))
{
	this->setObjectName("Context");
	this->addWidget(m_pixmap_frame);
	this->addWidget(m_text_frame);
	this->addWidget(m_richtext_frame);
}

StackedWidget::~StackedWidget()
{
	delete m_pixmap_frame;
	delete m_text_frame;
	delete m_richtext_frame;
}

void StackedWidget::setPixmap(QPixmap &pixmap)
{
	m_pixmap_frame->setStorePixmap(pixmap);
	this->setCurrentIndex(StackedWidget::IMAGE);

	QString s = QString("%1x%2 ").arg(pixmap.width()).arg(pixmap.height()) + QObject::tr("px");
	m_pixmap_frame->setMaskFrameText(s);
}

const QPixmap *StackedWidget::pixmap(void)
{
	return m_pixmap_frame->pixmap();
}

void StackedWidget::setText(QString &s)
{
	m_text_frame->setText(s);
	m_text_frame->setIndent(4);
	m_text_frame->setMaskFrameText(QString("%1 ").arg(s.count()) + QObject::tr("characters"));
	this->setCurrentIndex(StackedWidget::TEXT);
}

void StackedWidget::setRichText(QString &s, int count)
{
	m_richtext_frame->setText(s);
	m_richtext_frame->setTextFormat(Qt::RichText);
	m_richtext_frame->setMaskFrameText(QString("%1 ").arg(count) + QObject::tr("characters"));
	this->setCurrentIndex(StackedWidget::RICHTEXT);
}

QString StackedWidget::text(void)
{
	return m_text_frame->text();
}
