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
    m_text = s;

    if (s.startsWith("http://") || s.startsWith("ftp://") || s.startsWith("https://"))
        this->m_barnner->setTitle(QObject::tr("Link"));
    else if (QColor::isValidColor(s))
        this->m_barnner->setTitle(QObject::tr("Color"));
    else
        this->m_barnner->setTitle(QObject::tr("PlainText"));
}

void PasteItem::setRichText(QString richText, QString plainText)
{
    m_context->setRichText(richText, plainText);
    m_text = plainText;

    if (plainText.startsWith("http://") || plainText.startsWith("ftp://") || plainText.startsWith("https://"))
        this->m_barnner->setTitle(QObject::tr("Link"));
    else if (QColor::isValidColor(plainText.simplified().trimmed()))
        this->m_barnner->setTitle(QObject::tr("Color"));
    else
        this->m_barnner->setTitle(QObject::tr("RichText"));
}

bool PasteItem::setUrls(QList<QUrl> &urls)
{
    for (auto url : urls) {
        m_text += url.toString();
    }
    if (!m_context->setUrls(urls))
        return false;

    this->m_barnner->setTitle(QString("%1 ").arg(urls.count()) + QObject::tr("Files"));
    return true;
}

void PasteItem::setIcon(QPixmap pixmap)
{
    m_barnner->setIcon(pixmap);
}

void PasteItem::setTime(QDateTime &dateTime) {
    m_barnner->setTime(dateTime);
}

void PasteItem::resizeEvent(QResizeEvent *event) {
    QSize size = event->size();
    m_frame->resize(size);
    m_barnner->setFixedHeight(size.height()/5);

    QWidget::resizeEvent(event);
}

void PasteItem::mouseDoubleClickEvent(QMouseEvent *event) {
    this->copyData();

    QWidget::mouseDoubleClickEvent(event);
}

void PasteItem::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton) {
//        qDebug()<<"test";
//        this->menuData();
    }
    QWidget::mousePressEvent(event);
}
void PasteItem::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        this->copyData();
        break;
    case Qt::Key_Escape:
        emit this->hideWindow();
        break;
    }

    QWidget::keyPressEvent(event);
}

void PasteItem::copyData(void)
{
    emit this->hideWindow();

    QClipboard *clipboard = QApplication::clipboard();
    ItemData itemData = this->m_listwidget_item->data(Qt::UserRole).value<ItemData>();
    if (!itemData.image.isNull())
        itemData.mimeData->setImageData(itemData.image);

    clipboard->setMimeData(itemData.mimeData, QClipboard::Clipboard);
    clipboard->setMimeData(itemData.mimeData, QClipboard::Selection);

}

//void PasteItem::menuData(void){
//    QMenu menu =new QMenu;
//}
