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
