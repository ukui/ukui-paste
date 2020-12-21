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

#include "searchbar.h"

#include <QHBoxLayout>
#include <QResizeEvent>

#include <QApplication>
#include <QDebug>

/*搜索框的样式*/
LineEdit::LineEdit(QWidget *parent) : QLineEdit(parent)
{
    this->setFocusPolicy(Qt::ClickFocus);
    this->setContextMenuPolicy(Qt::NoContextMenu);
}

void LineEdit::focusInEvent(QFocusEvent *event)
{
    emit this->focusIn();
    QLineEdit::focusInEvent(event);
}

void LineEdit::focusOutEvent(QFocusEvent *event)
{
    emit this->focusOut();
    QLineEdit::focusOutEvent(event);
}
/*监听对搜索框的按键设置*/
bool LineEdit::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        switch (ke->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            emit this->selectItem();
            break;
        case Qt::Key_Escape:
            emit this->hideWindow();
            break;
        case Qt::Key_Tab:
            emit this->moveFocusPrevNext(false);
            /* event is done */
            return true;
        case Qt::Key_Backtab:
            emit this->moveFocusPrevNext(true);
            /* event is done */
            return true;
        }
    }

    return QLineEdit::event(event);
}

void LineEdit::hideEvent(QHideEvent *event)
{
    this->clear();
    QLineEdit::hideEvent(event);
}

PushButton::PushButton(QWidget *parent) : QPushButton(parent),
    m_label(new QLabel(this))
{
    this->setObjectName("PushButton");
    this->setAttribute(Qt::WA_StyledBackground);
    this->setFocusPolicy(Qt::ClickFocus);
    m_label->setAlignment(Qt::AlignCenter);
}
//void PushButton::updatePixmap(void)
//{
//    if (!m_pixmap.isNull()) {
//        QPixmap pixmap = m_pixmap.scaled(this->size()*0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation);
//        m_label->setPixmap(pixmap);
//    }
//}

//void PushButton::setPixmap(QPixmap pixmap)
//{
//    m_pixmap = pixmap;
//    this->updatePixmap();
//}

void PushButton::resizeEvent(QResizeEvent *event)
{
    m_label->setGeometry(QRect(0, 0, event->size().width(), event->size().height()));
//    this->updatePixmap();
    QPushButton::resizeEvent(event);
}

SearchBar::SearchBar(QWidget *parent, int width, int height) : QWidget(parent)
{
//    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setAttribute(Qt::WA_StyledBackground);
    this->setObjectName("SearchBar");
    this->setFixedSize(width, height);

    this->m_search_edit = new LineEdit(this);


    m_search_edit->setPlaceholderText(QObject::tr("Search"));
    m_search_edit->setTextMargins(10, 0, 0, 0);
    QObject::connect(m_search_edit, &LineEdit::focusOut, [this](void) {   });
    QObject::connect(m_search_edit, &LineEdit::focusIn, [this](void) {
    });
    QObject::connect(m_search_edit, &LineEdit::hideWindow, [this](void) {
        emit this->hideWindow();
    });
    QObject::connect(m_search_edit, &LineEdit::textChanged, [this](const QString &text) {
        emit this->textChanged(text);
    });
    QObject::connect(m_search_edit, &LineEdit::selectItem, [this](void) {
        emit this->selectItem();
    });
    QObject::connect(m_search_edit, &LineEdit::moveFocusPrevNext, [this](bool prev) {
        emit this->moveFocusPrevNext(prev);
    });

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(this->m_search_edit);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();

    this->setLayout(layout);
}
