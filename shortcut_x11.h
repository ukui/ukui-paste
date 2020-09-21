#ifndef SHORTCUT_X11_H
#define SHORTCUT_X11_H

#include <QTimer>
#include <QThread>
#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <X11/Xlibint.h>

class ShortcutPrivateX11 : public QThread
{
	Q_OBJECT

public:
	ShortcutPrivateX11(QObject *parent = nullptr);
	~ShortcutPrivateX11();

Q_SIGNALS:
	void activated(void);

private:
	static void callback(XPointer trash, XRecordInterceptData *data);

protected:
	void run();

public slots:
	void stop();

private:
	Display		*m_display;
	XRecordContext	m_context;
};


#endif // SHORTCUT_X11_H
