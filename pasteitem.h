#ifndef PASTEITEM_H
#define PASTEITEM_H

#include <QWidget>
#include <QFrame>
#include <QResizeEvent>

class PasteItem : public QWidget
{
	Q_OBJECT
public:
	explicit PasteItem(QWidget *parent = nullptr);

protected:
	void resizeEvent(QResizeEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);

private:
	QFrame	*m_frame;

Q_SIGNALS:
	void click(void);
};

#endif // PASTEITEM_H
