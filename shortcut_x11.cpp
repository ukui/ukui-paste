#include <QKeySequence>
#include <QDebug>

#include "shortcut_x11.h"

DoubleCtrlShortcut::DoubleCtrlShortcut(QObject *parent) : QThread(parent),
	m_display(nullptr),
	m_timer(new QTimer),
	m_isActive(false)
{
	QObject::connect(this->m_timer, &QTimer::timeout, [this](void) {
		this->m_isActive = false;
		this->m_timer->stop();
	});

	QObject::connect(this, &DoubleCtrlShortcut::keyPress, this, [this](int keyCode) {
		if (keyCode != 37 /* Left Control */) {
			this->m_isActive = false;
			return;
		}

		if (this->m_isActive) {
			emit this->activated();
		}

		this->m_isActive = true;
		if (this->m_timer->isActive())
			this->m_timer->stop();

		/* Record Press */
		this->m_timer->start(300);
	});

	this->start();
}

DoubleCtrlShortcut::~DoubleCtrlShortcut()
{
	this->stop();
	this->deleteLater();
}

void DoubleCtrlShortcut::run(void)
{
	Display *display = XOpenDisplay(nullptr);
	XRecordClientSpec clients = XRecordAllClients;
	XRecordRange *range = XRecordAllocRange();

	memset(range, 0, sizeof(XRecordRange));
	range->device_events.first = KeyPress;
	range->device_events.last = KeyPress;

	m_context = XRecordCreateContext(display, 0, &clients, 1, &range, 1);
	XFree(range);
	XSync(display, True);

	this->m_display = XOpenDisplay(nullptr);
	XRecordEnableContext(this->m_display, m_context, &DoubleCtrlShortcut::callback, reinterpret_cast<XPointer>(this));
}

void DoubleCtrlShortcut::stop()
{
	XRecordDisableContext(this->m_display, this->m_context);
	XFlush(this->m_display);
	XCloseDisplay(this->m_display);
}

void DoubleCtrlShortcut::callback(XPointer ptr, XRecordInterceptData *data)
{
	if (data->category == XRecordFromServer) {
		xEvent *event = reinterpret_cast<xEvent*>(data->data);
		switch (event->u.u.type) {
		case KeyPress:
			emit reinterpret_cast<DoubleCtrlShortcut*>(ptr)->keyPress(static_cast<unsigned char*>(data->data)[1]);
			break;
		default:
			break;
		}
	}

	XRecordFreeData(data);
}
