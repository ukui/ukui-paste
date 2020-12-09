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

#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QObject>
#include <QTimer>

#ifdef Q_OS_LINUX
#include "shortcut_x11.h"
#endif

#ifdef Q_OS_WIN
#include "shortcut_win.h"
#endif

class DoubleCtrlShortcut : public QObject
{
	Q_OBJECT

public:
	DoubleCtrlShortcut(QObject *parent = nullptr);

signals:
	void activated(void);

private:

#ifdef Q_OS_LINUX
	ShortcutPrivateX11	*m_shortcut;
#endif
#ifdef Q_OS_WIN
	ShortcutPrivateWin	*m_shortcut;
#endif
	QTimer			*m_timer;
	bool			m_isActive;
};

#endif // SHORTCUT_H
