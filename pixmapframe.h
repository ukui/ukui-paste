#ifndef PIXMAPFRAME_H
#define PIXMAPFRAME_H

#include <QLabel>
#include <QPixmap>
#include <QWidget>

class PixmapFrame : public QLabel
{
public:
	PixmapFrame(QWidget *parent = nullptr);

protected:
	void resizeEvent(QResizeEvent *event);
};

#endif // PIXMAPFRAME_H
