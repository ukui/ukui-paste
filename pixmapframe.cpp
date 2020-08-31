#include "pixmapframe.h"

#include <QResizeEvent>

PixmapFrame::PixmapFrame(QWidget *parent) : QLabel(parent)
{
	this->setStyleSheet("margin: 5px; padding: 0px;");
}

void PixmapFrame::resizeEvent(QResizeEvent *event)
{
	QSize size = event->size();
	const QPixmap *pixmap = this->pixmap();
	QPixmap pm = pixmap->scaled(size.width() - 10, size.height() - 10,
				    Qt::KeepAspectRatio, Qt::SmoothTransformation);
	this->setPixmap(pm);
	QLabel::resizeEvent(event);
}
