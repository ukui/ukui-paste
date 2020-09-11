#ifndef BARNNER_H
#define BARNNER_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QDateTime>

class Barnner : public QWidget
{
	Q_OBJECT
public:
	explicit Barnner(QWidget *parent = nullptr);

	void setIcon(QPixmap &pixmap)
	{
		m_pixmap = pixmap;
	}

	void setTitle(QString s)
	{
		this->m_text->setText(s);
		this->m_text->show();
	}

	void setTime(QDateTime &dateTime)
	{
		this->m_time->setText(dateTime.toString());
		this->m_text->show();
	}

	void setBackground(QRgb rgb);
	static QRgb averageColor(QPixmap *);

protected:
	void resizeEvent(QResizeEvent *event);

private:
	QLabel		*m_icon;
	QLabel		*m_text;
	QLabel		*m_time;

	QPixmap		m_pixmap;
};

#endif // BARNNER_H
