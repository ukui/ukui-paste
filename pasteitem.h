#ifndef PASTEITEM_H
#define PASTEITEM_H

#include "pasteitemcontext.h"
#include "pasteitembarnner.h"

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
	void setIcon(QPixmap);

private:
	void copyData(void);

protected:
	void resizeEvent(QResizeEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);

private:
	QWidget				*m_frame;
	QGraphicsDropShadowEffect	*m_frame_effect;

	/* barnner and context */
	Barnner				*m_barnner;
	StackedWidget			*m_context;

Q_SIGNALS:
	void hideWindow(void);
};

#endif // PASTEITEM_H
