#include "pixmapframe.h"

#include <QResizeEvent>

PixmapFrame::PixmapFrame(QWidget *parent) : QLabel(parent)
{
}

void PixmapFrame::resizeEvent(QResizeEvent *event)
{
	QSize size = event->size();
	const QPixmap *pixmap = this->pixmap();
	QPixmap pm = pixmap->scaled(size.width(), size.height(),
				    Qt::KeepAspectRatio, Qt::SmoothTransformation);
	this->setPixmap(pm);
	QLabel::resizeEvent(event);
}
