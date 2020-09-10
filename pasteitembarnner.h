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
	void setIcon(QPixmap &);

protected:
	void resizeEvent(QResizeEvent *event);

private:
	QLabel	*m_icon;

};

#endif // BARNNER_H
