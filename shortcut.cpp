#include <QDebug>

#include "shortcut.h"
#include "shortcut_win.h"

DoubleCtrlShortcut::DoubleCtrlShortcut(QObject *parent) : QObject(parent),
	m_timer(new QTimer),
	m_isActive(false)
{
#ifdef Q_OS_LINUX
	this->m_shortcut = new ShortcutPrivateX11();
	auto signal_address = &ShortcutPrivateX11::activated;
#elif Q_OS_WIN
	this->m_shortcut = new ShortcutPrivateWin();
	auto signal_address = &ShortcutPrivateWin::activated;
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
