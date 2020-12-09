/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

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

#ifdef Q_OS_WIN
QPixmap pixmapFromShellImageList(int iImageList, const SHFILEINFO &info);
#endif

class TextFrame : public QLabel
{
public:
	TextFrame(QWidget *parent = nullptr);
	~TextFrame();

	void setMaskFrameText(QString);
	void setBackgroundColor(QString);

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

class FileFrame : public TextFrame
{
public:
	FileFrame(QWidget *parent = nullptr);
	~FileFrame();

	QIcon getIcon(const QString &uri);
	bool setUrls(QList<QUrl> &);

	void setFilename(QString filename)
	{
		this->m_filename = filename;
	}

protected:
	void resizeEvent(QResizeEvent *event);

#ifdef Q_OS_WIN
	static QIcon getFileIcon(const QString &filename);
	static QIcon getDirIcon(const QString &filename);
	static QIcon getExecutableIcon(const QString &filename);
#endif

private:
	QList<QPair<QLabel *, QPixmap>> m_labels;
	QString				m_filename;
};

class StackedWidget : public QStackedWidget
{
public:
	StackedWidget(QWidget *parent = nullptr);
	~StackedWidget();

	void setPixmap(QPixmap &);
	void setText(QString &);
	void setRichText(QString &richText, QString &plainText);
	bool setUrls(QList<QUrl> &);

	enum V { IMAGE, TEXT, RICHTEXT, URLS };
	Q_ENUM(V)

private:
	PixmapFrame	*m_pixmap_frame;
	TextFrame	*m_text_frame;
	TextFrame	*m_richtext_frame;
	FileFrame	*m_file_frame;
};

#endif // PASTEITEMCONTEXT_H
