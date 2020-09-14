#include <QDebug>

#include "shortcut.h"

Shortcut::Shortcut(QObject *parent) : QObject(parent)
{
#ifdef Q_OS_LINUX
	this->m_shortcut = new DoubleCtrlShortcut(this);
	auto signalAddress = &DoubleCtrlShortcut::activated;
#endif
#ifdef Q_OS_WIN
	this->m_shortcut = new QxtGlobalShortcut(this);
	this->m_shortcut->setShortcut(QKeySequence("Ctrl+Shift+v"));
	auto signalAddress = &QxtGlobalShortcut::activated;
#endif
	QObject::connect(this->m_shortcut, signalAddress, this, [this](void) {
		emit this->activated();
	});
}
