#include "pasteitembarnner.h"

#include <QHBoxLayout>
#include <QResizeEvent>

Barnner::Barnner(QWidget *parent) : QWidget(parent),
	m_icon(new QLabel(this))
{
	this->m_icon->setAttribute(Qt::WA_TranslucentBackground);

	QHBoxLayout *hboxlayout = new QHBoxLayout(this);
	hboxlayout->addStretch();
	hboxlayout->addWidget(this->m_icon);
	hboxlayout->setSpacing(0);
	hboxlayout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(hboxlayout);
	this->setObjectName("Barnner");
	this->setStyleSheet("background-color: green;");
}

void Barnner::setIcon(QPixmap &pixmap)
{
	this->m_icon->setPixmap(pixmap);
	this->m_icon->show();
}

void Barnner::resizeEvent(QResizeEvent *event)
{
	QSize size = event->size();
	m_icon->setFixedHeight(size.height());
	m_icon->setFixedWidth(size.height()*1.8);

	if (m_icon->pixmap() && !m_icon->pixmap()->isNull())
		m_icon->pixmap()->scaledToHeight(size.height()*1.5,
						 Qt::SmoothTransformation);

	QWidget::resizeEvent(event);
}
