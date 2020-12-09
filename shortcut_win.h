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

#ifndef SHORTCUT_WIN_H
#define SHORTCUT_WIN_H

#include <QThread>

class ShortcutPrivateWin : public QThread
{
	Q_OBJECT
public:
	ShortcutPrivateWin();
	~ShortcutPrivateWin();

	void stop(void);

Q_SIGNALS:
	void activated(void);

protected:
	void run(void);

private:
	bool	stoped;
};

#endif // SHORTCUT_WIN_H
