#ifndef SHORTCUT_WIN_H
#define SHORTCUT_WIN_H

#include <QThread>

class ShortcutPrivate : public QThread
{
	Q_OBJECT
public:
	ShortcutPrivate();

Q_SIGNALS:
	void activated(void);

private:
	void run(void);
};

#endif // SHORTCUT_WIN_H
