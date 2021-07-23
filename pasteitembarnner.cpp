/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

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
    this->m_time->setStyleSheet(this->m_time->styleSheet()+"font-size: 11px;");

    QHBoxLayout *hboxlayout = new QHBoxLayout();

    hboxlayout->addWidget(this->m_icon);
    hboxlayout->addWidget(this->m_text);
    hboxlayout->addStretch();
    hboxlayout->addWidget(this->m_time);
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

/* 标题背景色 */
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
        if (m_pixmap.height() >= 16 || m_pixmap.width() >= 16) {
			m_icon->setFixedWidth(size.height()*1.45);
            mp = this->m_pixmap.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			this->m_icon->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		} else {
			m_icon->setFixedWidth(size.height()*0.8);
            mp = this->m_pixmap.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			m_icon->setAlignment(Qt::AlignCenter);
		}

//        this->setBackground(Barnner::averageColor(&this->m_pixmap));
		this->m_icon->setPixmap(mp);
	}

	QWidget::resizeEvent(event);
}

/* 时间 */
void Barnner::showEvent(QShowEvent *event)
{
	if (!this->m_datetime.isNull()) {
		qint64 currntSecs = QDateTime::currentDateTime().toSecsSinceEpoch();
		qint64 createSecs = this->m_datetime.toSecsSinceEpoch();
		qint64 period = currntSecs - createSecs;
		if (period >= 0) {
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
			} else if (secs > 10) {
				this->m_time->setText(QString("%1 ").arg(secs) + QObject::tr("secs ago"));
			} else {
				this->m_time->setText(QObject::tr("moment ago"));
			}
		}
	}

	QWidget::showEvent(event);
}
