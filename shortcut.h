#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QTimer>

#ifdef Q_OS_LINUX
#include "shortcut_x11.h"
#endif

#ifdef Q_OS_WIN
#include "3rd/qxtglobalshortcut5/gui/qxtglobalshortcut.h"
#endif

class Shortcut : public QObject
{
	Q_OBJECT

public:
	Shortcut(QObject *parent = nullptr);
	~Shortcut();

signals:
	void activated(void);

private:
#ifdef Q_OS_LINUX
	EventMonitor		*m_eventMonitor;
	QTimer			*m_timer;
	bool			m_isActive;
#endif
#ifdef Q_OS_WIN
	QxtGlobalShortcut	*m_shortcut;
#endif
};

#endif // SHORTCUT_H
