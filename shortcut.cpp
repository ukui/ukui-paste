#include <QDebug>

#include "shortcut.h"
#include "shortcut_win.h"

Shortcut::Shortcut(QObject *parent) : QObject(parent),
	m_timer(new QTimer),
	m_isActive(false)
{
#ifdef Q_OS_LINUX
	this->m_shortcut = new DoubleCtrlShortcut(this);
	auto signalAddress = &DoubleCtrlShortcut::activated;
	QObject::connect(this->m_shortcut, signalAddress, this, [this](void) {
		emit this->activated();
	});
#endif
#ifdef Q_OS_WIN
	this->m_shortcut = new ShortcutPrivate();
	QObject::connect(this->m_timer, &QTimer::timeout, [this](void) {
		this->m_isActive = false;
		this->m_timer->stop();
	});
	QObject::connect(this->m_shortcut, &ShortcutPrivate::activated, this, [this](void) {
		if (this->m_isActive)
			emit this->activated();

		this->m_isActive = true;
		if (this->m_timer->isActive())
			this->m_timer->stop();

		/* Record Press */
		this->m_timer->start(300);
	});
#endif
}
