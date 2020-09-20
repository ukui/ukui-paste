#include "shortcut_win.h"

#include <QtWin>
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <shellapi.h>
#include <comdef.h>
#include <commctrl.h>
#include <objbase.h>
#include <commoncontrols.h>
#include <QtWinExtras/QtWinExtras>
#include <QtWinExtras/QtWin>

ShortcutPrivate::ShortcutPrivate()
{
	this->start();
}

void ShortcutPrivate::run()
{
	RegisterHotKey(NULL, 1, MOD_CONTROL, 0);

	MSG msg;
	::memset(&msg, 0, sizeof(MSG));

	while (GetMessage(&msg, NULL, 0, 0) != 0) {
		if (msg.message == WM_HOTKEY) {
			emit this->activated();
		}
	}
}
