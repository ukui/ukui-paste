#include "pasteitembarnner.h"

#include <QHBoxLayout>
#include <QResizeEvent>

#include <QDebug>

Barnner::Barnner(QWidget *parent) : QWidget(parent),
	m_icon(new QLabel(this))
{
	this->m_icon->setAttribute(Qt::WA_TranslucentBackground);
	this->m_icon->setScaledContents(false);

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

	if (!this->m_pixmap.isNull()) {
		QPixmap mp;

		m_icon->setFixedHeight(size.height());
		if (m_pixmap.height() >= 128 || m_pixmap.width() >= 128) {
			m_icon->setFixedWidth(size.height()*1.45);
			mp = this->m_pixmap.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			this->m_icon->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		} else {
			m_icon->setFixedWidth(size.height()*0.8);
			mp = this->m_pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			m_icon->setAlignment(Qt::AlignCenter);
		}

		this->setBackground(Barnner::averageColor(&this->m_pixmap));
		this->m_icon->setPixmap(mp);
	}

	QWidget::resizeEvent(event);
}
