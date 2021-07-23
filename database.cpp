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
#include "database.h"

#include <QApplication>
#include <QSqlError>
#include <QBuffer>
#include <QSqlRecord>
#include <QDateTime>
#include <QThread>
#include <QDebug>

#define DEBUG() qDebug()<<__FILE__<<__func__<<__LINE__

Database::Database(QObject *parent) : QObject(parent) {
    /* 创建数据库 */
    this->m_db = QSqlDatabase::addDatabase("QSQLITE");
    this->m_db.setUserName("root");
    this->m_db.setPassword("QeErTyUiOp{]");
#ifdef Q_OS_LINUX
    /* 存放复制数据的地址 */
    this->m_db.setDatabaseName(QString(getenv("HOME")) + "/.cache/ukui-pastesDatabase.db");
#endif
    DEBUG() << this->m_db.databaseName();

    if (!this->m_db.open())
        DEBUG() << this->m_db.lastError();
}

Database::~Database() {
    this->m_db.close();
}

/* 查询是否存在表格 */
bool Database::isTableExist() {
    QSqlQuery query_item(QString("select * from sqlite_master where name = 'item'"), this->m_db);
    QSqlQuery query_data(QString("select * from sqlite_master where name = 'data'"), this->m_db);
    query_item.exec();
    query_data.exec();

    return query_item.next() && query_data.next();
}

/* 创建表单 */
void Database::createTable() {
    QSqlQuery query(this->m_db);

    if (!query.exec("create table if not exists item(id integer primary key autoincrement,"
                    " md5 blob,"
                    " imagedata blob,"
                    " icondata blob,"
                    " time integer)"))
        DEBUG() << query.lastError();

    if (!query.exec("create table if not exists data(id integer primary key autoincrement,"
                    " md5 blob,"
                    " formats text,"
                    " format_data blob)"))
        DEBUG() << query.lastError();

}

QByteArray Database::convertImage2Array(QImage image) {
    QByteArray imagedata;
    QBuffer buffer(&imagedata);

    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "png");
    buffer.close();

    return imagedata;
}

/* Used for lock database */
QMutex mutex;
/* 复制插入数据 */
void Database::insertPasteItem(ItemData *itemData) {
    /*
     * We need copy an new itemdata for thread, because QImage or QPixmap
     * and others is an private data for Class, can't share for another
     * thread, That is an workaround.
     */
    ItemData *itd = new ItemData(*itemData);
    itd->mimeData = new QMimeData;
    for (auto formats : itemData->mimeData->formats()) {
        itd->mimeData->setData(formats, itemData->mimeData->data(formats));
    }

    QThread *insert_thread = QThread::create([this, itd](void){
        QSqlQuery query(this->m_db);
        QMutexLocker locker(&mutex);

        query.prepare("insert into item (md5, imagedata, icondata, time) values (:md5, :imagedata, :icondata, :time);");
        query.bindValue(":md5", itd->md5);
        if (!itd->image.isNull()) {
            query.bindValue(":imagedata", Database::convertImage2Array(itd->image));
        }
        query.bindValue(":icondata", Database::convertImage2Array(itd->icon.toImage()));
        query.bindValue(":time", itd->time.toSecsSinceEpoch());

        if (!query.exec())
            DEBUG() << query.lastError();

        for (QString format : itd->mimeData->formats()) {
            QSqlQuery query(this->m_db);
            query.prepare("insert into data (md5, formats, format_data) values (:md5, :formats, :format_data);");
            query.bindValue(":md5", itd->md5);
            query.bindValue(":formats", format);
            query.bindValue(":format_data", itd->mimeData->data(format));

            if (!query.exec())
                DEBUG() << query.lastError();
        }

        delete itd->mimeData;
        delete itd;
    });

    insert_thread->start();
}

/* 查询是否存在数据 */
bool Database::isDataExist(){
    QSqlQuery isdata(QString("select * from data;"), this->m_db);
    isdata.exec();
    return isdata.next();
}

/* 根据分类展示 */
QList<ItemData *> Database::selectPasteItem(QString format) {
    QList<ItemData *> list;
    QSqlQuery query(this->m_db);
    /* 查询是否存在表单 */
    query.prepare("select * from item;");
    if (!query.exec())
        DEBUG() << query.lastError();

    while (query.next()) {
        ItemData *itemData = new ItemData;
        itemData->md5 = query.value("md5").toByteArray();
        itemData->image = QImage::fromData(query.value("imagedata").toByteArray());
        itemData->icon = QPixmap::fromImage(QImage::fromData(query.value("icondata").toByteArray()));
        itemData->time = QDateTime::fromSecsSinceEpoch(query.value("time").toUInt());

        itemData->mimeData = new QMimeData;
        QSqlQuery query_data(this->m_db);
        query_data.prepare("select * from data where md5 = x'" + itemData->md5.toHex() + "'");

        if(format == "1"){
            query_data.prepare("select * from data;");
        }else{
            query_data.prepare("select * from data where formats = '"+format+"' ;");
        }

        if (!query_data.exec())
            DEBUG() << query.lastError();

        while (query_data.next()) {
            QString mimeType = query_data.value("formats").toString();
            QByteArray data = query_data.value("format_data").toByteArray();
            itemData->mimeData->setData(mimeType, data);
        }

        if (!itemData->image.isNull())
            itemData->mimeData->setImageData(itemData->image);

        list.push_back(itemData);
    }

    while (query.next()){
         qDebug() << query.value(0).toString() << query.value(1).toString() << query.value(2).toString();
    }
    return list;

}

/* 删除所有数据 */
void Database::deleleAllPasteItem() {
    QThread *delete_thread = QThread::create([this](void) {
    QSqlQuery query(this->m_db);
    QMutexLocker locker(&mutex);
    query.prepare("delete from data");
    if (!query.exec())
        DEBUG() << query.lastError();
    });

    delete_thread->start();
}

/* 根据md5删除数据 */
void Database::delelePasteItem(QByteArray md5) {
    QThread *delete_thread = QThread::create([this, md5](void) {
        QSqlQuery query(this->m_db);
        QMutexLocker locker(&mutex);

        query.prepare("delete from item where md5 = x'" + md5.toHex() + "'");
        if (!query.exec())
            DEBUG() << query.lastError();
        query.prepare("delete from data where md5 = x'" + md5.toHex() + "'");
        if (!query.exec())
            DEBUG() << query.lastError();
    });

    delete_thread->start();
}

QList<ItemData *> Database::loadData(void) {
    QList<ItemData *> list;
    QSqlQuery query(this->m_db);

    query.prepare("select * from item;");
    if (!query.exec())
        DEBUG() << query.lastError();
    /* 增加数据格式 */
    while (query.next()) {
        ItemData *itemData = new ItemData;
        itemData->md5 = query.value("md5").toByteArray();
        itemData->image = QImage::fromData(query.value("imagedata").toByteArray());
        itemData->icon = QPixmap::fromImage(QImage::fromData(query.value("icondata").toByteArray()));
        itemData->time = QDateTime::fromSecsSinceEpoch(query.value("time").toUInt());

        itemData->mimeData = new QMimeData;
        QSqlQuery query_data(this->m_db);
        query_data.prepare("select * from data where md5 = x'" + itemData->md5.toHex() + "'");
        if (!query_data.exec())
            DEBUG() << query.lastError();

        while (query_data.next()) {
            QString mimeType = query_data.value("formats").toString();
            QByteArray data = query_data.value("format_data").toByteArray();
            itemData->mimeData->setData(mimeType, data);
        }

        if (!itemData->image.isNull())
            itemData->mimeData->setImageData(itemData->image);

        list.push_back(itemData);
    }

    return list;
}
