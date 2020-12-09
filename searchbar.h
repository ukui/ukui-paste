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

#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QLabel>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QPropertyAnimation>

class LineEdit : public QLineEdit
{
	Q_OBJECT
public:
    LineEdit(QWidget *parent = nullptr/*, int parent_width = 0, int parent_height = 0*/);

protected:
	void focusInEvent(QFocusEvent *event);
	void focusOutEvent(QFocusEvent *event);
	void hideEvent(QHideEvent *event);
	bool event(QEvent * event);

Q_SIGNALS:
	void focusIn(void);
	void focusOut(void);
	void hideWindow(void);
	void selectItem(void);
	void moveFocusPrevNext(bool);

private:

};

class PushButton : public QPushButton
{
public:
    PushButton(QWidget *parent = nullptr);
	void updatePixmap(void);
	void setPixmap(QPixmap pixmap);

protected:
	void resizeEvent(QResizeEvent *event);

private:
    QLabel	*m_label;
    QPixmap	m_pixmap;
};

class SearchBar : public QWidget
{
	Q_OBJECT
public:
	SearchBar(QWidget *parent = nullptr, int width = 0, int height = 0);

private:
	LineEdit		*m_search_edit;
	PushButton		*m_search_button;

Q_SIGNALS:
	void moveFocusPrevNext(bool);
	void selectItem(void);
	void hideWindow(void);
	void textChanged(const QString &);
};

#endif // SEARCHBAR_H
