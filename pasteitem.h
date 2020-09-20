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
	QPixmap		icon;
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
	void setRichText(QString richText, QString plainText);
	void setUrls(QList<QUrl> &);
	void setIcon(QPixmap);
	void setTime(QDateTime &);
	void copyData(void);

	const QString &text(void)
	{
		return m_text;
	}

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

	/* text for search */
	QString				m_text;

Q_SIGNALS:
	void hideWindow(void);
};

#endif // PASTEITEM_H
