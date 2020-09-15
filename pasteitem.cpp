#include "pasteitem.h"

#include <QClipboard>
#include <QApplication>
#include <QVBoxLayout>
#include <QMimeData>
#include <QDebug>

PasteItem::PasteItem(QWidget *parent, QListWidgetItem *item) : QWidget(parent),
	m_frame(new QWidget(this)),
	m_frame_effect(new QGraphicsDropShadowEffect(this)),
	m_barnner(new Barnner(this->m_frame)),
	m_context(new StackedWidget(this->m_frame)),
	m_listwidget_item(item)
{
	this->setFocusPolicy(Qt::StrongFocus);
	this->setAttribute(Qt::WA_TranslucentBackground);

	m_frame_effect->setOffset(0, 0);
	m_frame_effect->setColor(Qt::gray);
	m_frame_effect->setBlurRadius(8);
	m_frame->setGraphicsEffect(m_frame_effect);
	m_frame->setObjectName("PasteItemFrame");

	QVBoxLayout *vboxlayout = new QVBoxLayout();
	vboxlayout->addWidget(m_barnner);
	vboxlayout->addWidget(m_context);
	vboxlayout->setSpacing(0);
	vboxlayout->setContentsMargins(5, 5, 5, 5);

	m_frame->setLayout(vboxlayout);
	m_frame->show();
}

void PasteItem::setImage(QImage &image)
{
	QPixmap pixmap = QPixmap::fromImage(image);
	m_context->setPixmap(pixmap);
	this->m_barnner->setTitle(QObject::tr("Image"));
}

void PasteItem::setPlainText(QString s)
{
	m_context->setText(s);
	this->m_barnner->setTitle(QObject::tr("PlainText"));
}

void PasteItem::setRichText(QString s, int count)
{
	m_context->setRichText(s, count);
	this->m_barnner->setTitle(QObject::tr("RichText"));
}

void PasteItem::setIcon(QPixmap pixmap)
{
	m_barnner->setIcon(pixmap);
}

QPixmap PasteItem::icon(void)
{
	return m_barnner->icon();
}

void PasteItem::setTime(QDateTime &dateTime)
{
	m_barnner->setTime(dateTime);
}

void PasteItem::resizeEvent(QResizeEvent *event)
{
	QSize size = event->size();
	m_frame->resize(size);
	m_barnner->setFixedHeight(size.height()/5);

	QWidget::resizeEvent(event);
}

void PasteItem::mouseDoubleClickEvent(QMouseEvent *event)
{
	this->copyData();

	QWidget::mouseDoubleClickEvent(event);
}

void PasteItem::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Return:
	case Qt::Key_Enter:
		this->copyData();
		break;
	case Qt::Key_Escape:
		emit this->hideWindow(false);
		break;
	}

	QWidget::keyPressEvent(event);
}

void PasteItem::copyData(void)
{
	QClipboard *clipboard = QApplication::clipboard();
	ItemData itemData = this->m_listwidget_item->data(Qt::UserRole).value<ItemData>();

	/* That is image, copy it */
	if (itemData.type == ItemData::IMAGE) {
		clipboard->setImage(itemData.image);
	}
	/* That is html, copy it */
	if (itemData.type == ItemData::HTML) {
		QMimeData *mimeData = new QMimeData;
		mimeData->setHtml(itemData.html);
		mimeData->setText(itemData.text);
		if (!itemData.image.isNull())
			mimeData->setImageData(itemData.image);
		clipboard->setMimeData(mimeData);
	}
	/* That is urls, copy it */
	if (itemData.type == ItemData::URLS) {
		QMimeData *mimeData = new QMimeData;
		mimeData->setUrls(itemData.urls);
		clipboard->setMimeData(mimeData);
	}
	/* That is text, copy it */
	if (itemData.type == ItemData::TEXT) {
		clipboard->setText(itemData.text);
	}

	emit this->hideWindow(true);
}
