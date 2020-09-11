#ifndef PASTEITEM_H
#define PASTEITEM_H

#include "pasteitemcontext.h"
#include "pasteitembarnner.h"

#include <QListWidgetItem>
#include <QWidget>
#include <QResizeEvent>
#include <QLabel>
#include <QGraphicsDropShadowEffect>

struct ItemData : QObjectUserData
{
	enum DATA_TYPE { HTML, IMAGE, URLS, TEXT };

	DATA_TYPE	type;

	QImage		image;
	QList<QUrl>	urls;
	QString		html;
	QString		text;
};
Q_DECLARE_METATYPE(ItemData);

class PasteItem : public QWidget
{
	Q_OBJECT
public:
	explicit PasteItem(QWidget *parent = nullptr, QListWidgetItem *item = nullptr);
	void setImage(QImage &);
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

	/* scroll list widget item */
	QListWidgetItem			*m_listwidget_item;

Q_SIGNALS:
	void hideWindow(void);
};

#endif // PASTEITEM_H
