#ifndef BARNNER_H
#define BARNNER_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>

class Barnner : public QWidget
{
	Q_OBJECT
public:
	explicit Barnner(QWidget *parent = nullptr);

	void setIcon(QPixmap &pixmap)
	{
		m_pixmap = pixmap;
	}

	void setBackground(QRgb rgb);
	static QRgb averageColor(QPixmap *);

protected:
	void resizeEvent(QResizeEvent *event);

private:
	QLabel	*m_icon;
	QPixmap	m_pixmap;
};

#endif // BARNNER_H
