#ifndef SHORTCUT_WIN_H
#define SHORTCUT_WIN_H

#include <QThread>

class ShortcutPrivateWin : public QThread
{
	Q_OBJECT
public:
	ShortcutPrivateWin();
	~ShortcutPrivateWin();

	void stop(void);

Q_SIGNALS:
	void activated(void);

protected:
	void run(void);

private:
	bool	stoped;
};

#endif // SHORTCUT_WIN_H
