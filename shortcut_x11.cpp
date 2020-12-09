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

#include <QKeySequence>
#include <QDebug>

#include "shortcut_x11.h"

ShortcutPrivateX11::ShortcutPrivateX11(QObject *parent) : QThread(parent),
	m_display(nullptr)
{
	this->start();
}

ShortcutPrivateX11::~ShortcutPrivateX11()
{
	this->stop();
	this->deleteLater();
}

void ShortcutPrivateX11::run(void)
{
	Display *display = XOpenDisplay(nullptr);
	XRecordClientSpec clients = XRecordAllClients;
	XRecordRange *range = XRecordAllocRange();

	memset(range, 0, sizeof(XRecordRange));
	range->device_events.first = KeyPress;
	range->device_events.last = KeyPress;

	m_context = XRecordCreateContext(display, 0, &clients, 1, &range, 1);
	XFree(range);
	XSync(display, True);

	this->m_display = XOpenDisplay(nullptr);
	XRecordEnableContext(this->m_display, m_context, &ShortcutPrivateX11::callback, reinterpret_cast<XPointer>(this));
}

void ShortcutPrivateX11::stop()
{
	XRecordDisableContext(this->m_display, this->m_context);
	XFlush(this->m_display);
	XCloseDisplay(this->m_display);
}

void ShortcutPrivateX11::callback(XPointer ptr, XRecordInterceptData *data)
{
	if (data->category == XRecordFromServer) {
		xEvent *event = reinterpret_cast<xEvent*>(data->data);
		switch (event->u.u.type) {
		case KeyPress:
			if (static_cast<unsigned char*>(data->data)[1] == 37  /* Left  Control */||
			    static_cast<unsigned char*>(data->data)[1] == 105 /* Right Control */)
				emit reinterpret_cast<ShortcutPrivateX11*>(ptr)->activated();
			break;
		default:
			break;
		}
	}

	XRecordFreeData(data);
}
