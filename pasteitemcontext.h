#ifndef PASTEITEMCONTEXT_H
#define PASTEITEMCONTEXT_H

#include <QLabel>
#include <QPixmap>
#include <QWidget>
#include <QStackedWidget>
#include <QPainter>
#include <QStyleOption>

#define LABEL_HEIGHT	30

class TextFrame : public QLabel
{
public:
	TextFrame(QWidget *parent = nullptr);
	~TextFrame();

	void setMaskFrameText(QString);

protected:
	void resizeEvent(QResizeEvent *event);

private:
	QLabel	*m_mask_label;
};

class PixmapFrame : public TextFrame
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
	~StackedWidget();

	void setPixmap(QPixmap &);
	void setText(QString &);
	void setRichText(QString &, int);

	const QPixmap *pixmap(void);
	QString text(void);

	enum V { IMAGE, TEXT, RICHTEXT, URLS };
	Q_ENUM(V)

private:
	PixmapFrame	*m_pixmap_frame;
	TextFrame	*m_text_frame;
	TextFrame	*m_richtext_frame;
};

#endif // PASTEITEMCONTEXT_H
