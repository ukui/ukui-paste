#ifndef SHORTCUT_X11_H
#define SHORTCUT_X11_H

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


#endif // SHORTCUT_X11_H
