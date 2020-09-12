#include <QDebug>

#include "shortcut.h"

Shortcut::Shortcut(QObject *parent) : QObject(parent)
{
#ifdef Q_OS_LINUX
	this->m_timer = new QTimer;

	QObject::connect(this->m_timer, &QTimer::timeout, [this](void) {
		this->m_isActive = false;
	});

	this->m_eventMonitor = new EventMonitor();
	QObject::connect(this->m_eventMonitor, &EventMonitor::keyPress, this, [this](void) {
		if (this->m_isActive) {
			emit this->activated();
		}

		this->m_isActive = true;
		if (this->m_timer->isActive())
			this->m_timer->stop();

		/* Record Press */
		this->m_timer->start(400);
	});
	this->m_eventMonitor->start();
#endif
#ifdef Q_OS_WIN
	this->m_shortcut = new QxtGlobalShortcut(this);
	this->m_shortcut->setShortcut(QKeySequence("Ctrl+Shift+v"));
	QObject::connect(this->m_shortcut, &QxtGlobalShortcut::activated, this, [this](void) {
		emit this->activated();
	});
#endif
}

Shortcut::~Shortcut()
{
#ifdef Q_OS_LINUX
	this->m_eventMonitor->stop();
	this->m_eventMonitor->deleteLater();
#endif
}
