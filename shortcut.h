#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QTimer>

#include <QThread>
#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <X11/Xlibint.h>

class EventMonitor : public QThread
{
	Q_OBJECT

public:
	EventMonitor(QObject *parent = nullptr);

signals:
	void keyPress(int code);

protected:
	static void callback(XPointer trash, XRecordInterceptData *data);
	void handleRecordEvent(XRecordInterceptData *);
	void run();

public slots:
	void stop();

private:
	bool		m_isPress;
	Display		*m_display;
	XRecordContext	m_context;
};

class Shortcut : public QObject
{
	Q_OBJECT

public:
	Shortcut(QObject *parent = nullptr);
	~Shortcut();

signals:
	void activated(void);

private:
	EventMonitor	*m_eventMonitor;
	QTimer		*m_timer;
	bool		m_isActive;
};

#endif // SHORTCUT_H
