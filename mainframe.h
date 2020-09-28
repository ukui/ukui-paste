#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <QWidget>

class MainFrame : public QWidget
{
	Q_OBJECT

public:
	MainFrame(QWidget *parent = nullptr);

protected:
	bool event(QEvent *event);

signals:
	void moveFocusPrevNext(bool);
};

#endif // MAINFRAME_H
