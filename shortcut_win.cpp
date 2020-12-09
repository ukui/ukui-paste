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

#include "shortcut_win.h"

#include <QtWin>
#include <winuser.h>

ShortcutPrivateWin::ShortcutPrivateWin() :
	stoped(false)
{
	this->start();
}

ShortcutPrivateWin::~ShortcutPrivateWin()
{
	this->stop();
}

void ShortcutPrivateWin::run()
{
	RegisterHotKey(NULL, 1, MOD_CONTROL, 0);

	MSG msg;
	::memset(&msg, 0, sizeof(MSG));

	while ((GetMessage(&msg, NULL, 0, 0) != 0) && !stoped) {
		if (msg.message == WM_HOTKEY) {
			emit this->activated();
		}
	}
}

void ShortcutPrivateWin::stop()
{
	stoped = true;
	UnregisterHotKey(NULL, 1);
}
