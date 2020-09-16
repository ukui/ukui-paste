#ifndef PASTEITEM_H
#define PASTEITEM_H

#include "pasteitemcontext.h"
#include "pasteitembarnner.h"

#include <QListWidgetItem>
#include <QWidget>
#include <QResizeEvent>
#include <QLabel>
#include <QByteArray>
#include <QGraphicsDropShadowEffect>
#include <QMimeData>

struct ItemData : QObjectUserData
{
	QMimeData	*mimeData;
	QImage		image;
	QByteArray	md5;

	/* The time of data create */
	QDateTime	time;
};
Q_DECLARE_METATYPE(ItemData);

class PasteItem : public QWidget
{
	Q_OBJECT
public:
	explicit PasteItem(QWidget *parent = nullptr, QListWidgetItem *item = nullptr);
	void setImage(QImage &);
	void setPlainText(QString);
	void setRichText(QString, int);
	void setUrls(QList<QUrl> &);
	void setIcon(QPixmap);
	void setTime(QDateTime &);

	QPixmap icon(void);

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
	void hideWindow(bool);
};

#endif // PASTEITEM_H
