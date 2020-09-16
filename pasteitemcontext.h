#ifndef PASTEITEMCONTEXT_H
#define PASTEITEMCONTEXT_H

#include <QLabel>
#include <QPixmap>
#include <QWidget>
#include <QStackedWidget>
#include <QPainter>
#include <QStyleOption>

#ifdef Q_OS_WIN
#include <QtWin>
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <shellapi.h>
#include <comdef.h>
#include <commctrl.h>
#include <objbase.h>
#include <commoncontrols.h>
#endif

#define LABEL_HEIGHT	30

QPixmap pixmapFromShellImageList(int iImageList, const SHFILEINFO &info);

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

class FileFrame : public QWidget
{
public:
	FileFrame(QWidget *parent = nullptr);
	QIcon getIcon(const QString &uri);
	void setUrls(QList<QUrl> &);

#ifdef Q_OS_WIN
	static QIcon getFileIcon(const QString &filename);
	static QIcon getDirIcon(const QString &filename);
	static QIcon getExecutableIcon(const QString &filename);
#endif
};

class StackedWidget : public QStackedWidget
{
public:
	StackedWidget(QWidget *parent = nullptr);
	~StackedWidget();

	void setPixmap(QPixmap &);
	void setText(QString &);
	void setRichText(QString &, int);
	void setUrls(QList<QUrl> &);

	enum V { IMAGE, TEXT, RICHTEXT, URLS };
	Q_ENUM(V)

private:
	PixmapFrame	*m_pixmap_frame;
	TextFrame	*m_text_frame;
	TextFrame	*m_richtext_frame;
	FileFrame	*m_file_frame;
};

#endif // PASTEITEMCONTEXT_H
