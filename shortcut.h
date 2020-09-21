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
