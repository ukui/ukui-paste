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

#include <QDebug>

#include "shortcut.h"

DoubleCtrlShortcut::DoubleCtrlShortcut(QObject *parent) : QObject(parent),
	m_timer(new QTimer),
	m_isActive(false)
{
#ifdef Q_OS_LINUX
	this->m_shortcut = new ShortcutPrivateX11();
	auto signal_address = &ShortcutPrivateX11::activated;
#endif

    QObject::connect(this->m_timer, &QTimer::timeout, [this](void) {
		this->m_isActive = false;
		this->m_timer->stop();
	});
	QObject::connect(this->m_shortcut, signal_address, this, [this](void) {
		if (this->m_isActive)
			emit this->activated();

		this->m_isActive = true;
		if (this->m_timer->isActive())
			this->m_timer->stop();

		/* Record Press */
		this->m_timer->start(300);
	});
}
