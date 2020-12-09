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

#ifndef SHORTCUT_X11_H
#define SHORTCUT_X11_H

#include <QTimer>
#include <QThread>

#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <X11/Xlibint.h>

/* Needs by QMetaType::Bool, They are conflict */
#undef Bool

class ShortcutPrivateX11 : public QThread
{
	Q_OBJECT

public:
	ShortcutPrivateX11(QObject *parent = nullptr);
	~ShortcutPrivateX11();

Q_SIGNALS:
	void activated(void);

private:
	static void callback(XPointer trash, XRecordInterceptData *data);

protected:
	void run();

public slots:
	void stop();

private:
	Display		*m_display;
	XRecordContext	m_context;
};


#endif // SHORTCUT_X11_H
