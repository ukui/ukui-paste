#ifndef SHORTCUT_X11_H
#define SHORTCUT_X11_H

#include <QTimer>
#include <QThread>
#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <X11/Xlibint.h>

class DoubleCtrlShortcut : public QThread
{
	Q_OBJECT

public:
	DoubleCtrlShortcut(QObject *parent = nullptr);
	~DoubleCtrlShortcut();

private: signals:
	void keyPress(int code);

public: signals:
	void activated(void);

protected:
	static void callback(XPointer trash, XRecordInterceptData *data);
	void run();

public slots:
	void stop();

private:
	Display		*m_display;
	XRecordContext	m_context;

	QTimer		*m_timer;
	bool		m_isActive;
};


#endif // SHORTCUT_X11_H
