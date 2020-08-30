#ifndef PASTEITEM_H
#define PASTEITEM_H

#include "pixmapframe.h"

#include <QWidget>
#include <QFrame>
#include <QResizeEvent>
#include <QLabel>

class PasteItem : public QWidget
{
	Q_OBJECT
public:
	explicit PasteItem(QWidget *parent = nullptr);
	void setImage(QImage &image);
	void setPlainText(QString s);
	void setRichText(QString s);

protected:
	void resizeEvent(QResizeEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);

private:
	QFrame		*m_frame;
	PixmapFrame	*m_pixmap;
	QLabel		*m_plaintext;
	QLabel		*m_richtext;

Q_SIGNALS:
	void click(void);
};

#endif // PASTEITEM_H
