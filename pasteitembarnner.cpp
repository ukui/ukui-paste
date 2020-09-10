#include "pasteitembarnner.h"

#include <QHBoxLayout>
#include <QResizeEvent>

#include <QDebug>

Barnner::Barnner(QWidget *parent) : QWidget(parent),
	m_icon(new QLabel(this))
{
	this->m_icon->setAttribute(Qt::WA_TranslucentBackground);
	this->setAttribute(Qt::WA_StyledBackground, true);
	QHBoxLayout *hboxlayout = new QHBoxLayout();
	hboxlayout->addStretch();
	hboxlayout->addWidget(this->m_icon);
	hboxlayout->setSpacing(0);
	hboxlayout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(hboxlayout);
	this->setObjectName("Barnner");
}

void Barnner::setBackground(QRgb rgb)
{
	QString s = QString("background-color: rgb(%1, %2, %3);")
				.arg(qRed(rgb))
				.arg(qGreen(rgb))
				.arg(qBlue(rgb));
	this->setStyleSheet(s);
}

QRgb Barnner::averageColor(QPixmap *pixmap)
{
	float r = 0, g = 0, b = 0;
	QColor color;

	if (!pixmap || pixmap->isNull())
		return qRgb(255, 255, 255);

	QImage image = pixmap->toImage();
	for (int i = 0; i < image.width(); i++) {
		for (int j = 0; j < image.height(); j++) {
			color = QColor(image.pixel(i, j));
			r += color.red();
			g += color.green();
			b += color.blue();
		}
	}

	int count = image.width() * image.height();
	r /= count;
	g /= count;
	b /= count;

	return qRgb((int)r, (int)g, (int)b);
}

void Barnner::resizeEvent(QResizeEvent *event)
{
	QSize size = event->size();
	qDebug() << size;

	if (!this->m_pixmap.isNull()) {
		m_icon->setFixedHeight(size.height());
		m_icon->setFixedWidth(size.height()*1.45);
		this->m_pixmap.scaledToHeight(size.height()*1.5,
					      Qt::SmoothTransformation);
		this->setBackground(Barnner::averageColor(&this->m_pixmap));
		this->m_icon->setPixmap(this->m_pixmap);
	}

	QWidget::resizeEvent(event);
}
