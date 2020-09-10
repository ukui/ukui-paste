#ifndef PASTEITEMCONTEXT_H
#define PASTEITEMCONTEXT_H

#include <QLabel>
#include <QPixmap>
#include <QWidget>
#include <QStackedWidget>
#include <QPainter>
#include <QStyleOption>

class PixmapFrame : public QLabel
{
public:
	PixmapFrame(QWidget *parent = nullptr);
	void setStorePixmap(QPixmap pixmap)
	{
		m_pixmap = pixmap;
	}

private:
	QPixmap		m_pixmap;

protected:
	void resizeEvent(QResizeEvent *event);
};

class StackedWidget : public QStackedWidget
{
public:
	StackedWidget(QWidget *parent = nullptr);

	void setPixmap(QPixmap &);
	void setText(QString &);

	const QPixmap *pixmap(void);
	QString text(void);

	enum V { IMAGE, TEXT };
	Q_ENUM(V)

private:
	PixmapFrame	*m_pixmap_frame;
	QLabel		*m_text_frame;
};

#endif // PASTEITEMCONTEXT_H
