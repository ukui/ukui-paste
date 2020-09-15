#include "pasteitembarnner.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>

#include <QDebug>

Barnner::Barnner(QWidget *parent) : QWidget(parent),
	m_icon(new QLabel(this)),
	m_text(new QLabel(this)),
	m_time(new QLabel(this))
{
	this->setObjectName("Barnner");
	this->setAttribute(Qt::WA_StyledBackground, true);

	this->m_icon->setScaledContents(false);
	this->m_text->setStyleSheet(this->m_text->styleSheet()+"font-size: 22px;");
	this->m_time->setStyleSheet(this->m_time->styleSheet()+"font-size: 11px; color: rgba(255, 255, 255, 0.7);");

	QVBoxLayout *vboxlayout = new QVBoxLayout();
	vboxlayout->setSpacing(0);
	vboxlayout->addStretch();
	vboxlayout->setContentsMargins(20, 0, 0, 0);
	vboxlayout->addWidget(this->m_text);
	vboxlayout->addWidget(this->m_time);
	vboxlayout->addStretch();

	QHBoxLayout *hboxlayout = new QHBoxLayout();
	hboxlayout->addLayout(vboxlayout);
	hboxlayout->addStretch();
	hboxlayout->addWidget(this->m_icon);
	hboxlayout->setSpacing(0);
	hboxlayout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(hboxlayout);
}

Barnner::~Barnner()
{
	delete m_time;
	delete m_text;
	delete m_icon;
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

void Barnner::showEvent(QShowEvent *event)
{
	if (!this->m_datetime.isNull()) {
		qint64 currntSecs = QDateTime::currentDateTime().toSecsSinceEpoch();
		qint64 createSecs = this->m_datetime.toSecsSinceEpoch();
		qint64 period = currntSecs - createSecs;
		if (period > 0) {
			int months  = period / (30 * 24 * 60 * 60);
			int days    = period / (24 * 60 * 60);
			int hours   = period / (60 * 60);
			int minutes = period / 60;
			int secs    = period;

			if (months) {
				this->m_time->setText(QString("%1 ").arg(months) + QObject::tr("months ago"));
			} else if (days) {
				this->m_time->setText(QString("%1 ").arg(days) + QObject::tr("days ago"));
			} else if (hours) {
				this->m_time->setText(QString("%1 ").arg(hours) + QObject::tr("hours ago"));
			} else if (minutes) {
				this->m_time->setText(QString("%1 ").arg(minutes) + QObject::tr("minutes ago"));
			} else if (secs) {
				this->m_time->setText(QString("%1 ").arg(secs) + QObject::tr("secs ago"));
			}
		}
	}

	QWidget::showEvent(event);
}
