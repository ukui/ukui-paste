#ifndef PASTEITEM_H
#define PASTEITEM_H

#include "pixmapframe.h"

#include <QWidget>
#include <QResizeEvent>
#include <QLabel>
#include <QGraphicsDropShadowEffect>

class PasteItem : public QWidget
{
	Q_OBJECT
public:
	explicit PasteItem(QWidget *parent = nullptr);
	void setImage(QImage &image);
	void setPlainText(QString s);

private:
	void copyData(void);

protected:
	void resizeEvent(QResizeEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);

private:
	QWidget				*m_frame;
	QGraphicsDropShadowEffect	*m_frame_effect;
	PixmapFrame			*m_pixmap;
	QLabel				*m_plaintext;

Q_SIGNALS:
	void hideWindow(void);
};

#endif // PASTEITEM_H
