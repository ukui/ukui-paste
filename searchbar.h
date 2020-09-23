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
	LineEdit(QWidget *parent = nullptr, int parent_width = 0, int parent_height = 0);

protected:
	void focusInEvent(QFocusEvent *event);
	void focusOutEvent(QFocusEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void hideEvent(QHideEvent *event);

Q_SIGNALS:
	void focusIn(void);
	void focusOut(void);
	void hideWindow(void);
	void selectItem(void);

private:
	QPropertyAnimation	*m_zoom_animation;
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
	void selectItem(void);
	void hideWindow(void);
	void textChanged(const QString &);
};

#endif // SEARCHBAR_H
