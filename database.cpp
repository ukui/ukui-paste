#include "database.h"

#include <QDebug>
#include <QApplication>
#include <QSqlError>
#include <QBuffer>
#include <QSqlRecord>
#include <QDateTime>

Database::Database(QObject *parent) : QObject(parent)
{
	this->m_db = QSqlDatabase::addDatabase("QSQLITE");
	this->m_db.setUserName("root");
	this->m_db.setPassword("QeErTyUiOp{]");
	this->m_db.setDatabaseName(QCoreApplication::applicationDirPath() + "/" + ".PasteDatabase.db");
	qDebug() << this->m_db.databaseName();

	if (!this->m_db.open())
		qDebug() << this->m_db.lastError();
}

Database::~Database()
{
	this->m_db.close();
}

bool Database::isTableExist()
{
	QSqlQuery query_item(QString("select * from sqlite_master where name = 'item'"), this->m_db);
	QSqlQuery query_data(QString("select * from sqlite_master where name = 'data'"), this->m_db);
	query_item.exec();
	query_data.exec();

	return query_item.next() && query_data.next();
}

void Database::createTable()
{
	QSqlQuery query(this->m_db);

	if (!query.exec("create table if not exists item(id integer primary key autoincrement, md5 blob, imagedata blob, time integer)"))
		qDebug() << query.lastError();

	if (!query.exec("create table if not exists data(id integer primary key autoincrement, md5 blob, formats text, format_data blob)"))
		qDebug() << query.lastError();
}

void Database::insertPasteItem(ItemData *itemData)
{
	QSqlQuery query(this->m_db);

	query.prepare("insert into item (md5, imagedata, time) values (:md5, :imagedata, :time);");
	query.bindValue(":md5", itemData->md5);
	if (!itemData->image.isNull()) {
		QByteArray imagedata;
		QBuffer buffer(&imagedata);
		buffer.open(QIODevice::WriteOnly);
		itemData->image.save(&buffer, "png");
		buffer.close();

		query.bindValue(":imagedata", imagedata);
	}
	query.bindValue(":time", itemData->time.toSecsSinceEpoch());

	if (!query.exec())
		qDebug() << query.lastError();

	for (QString format : itemData->mimeData->formats()) {
		QSqlQuery query(this->m_db);
		query.prepare("insert into data (md5, formats, format_data) values (:md5, :formats, :format_data);");
		query.bindValue(":md5", itemData->md5);
		query.bindValue(":formats", format);
		query.bindValue(":format_data", itemData->mimeData->data(format));

		if (!query.exec())
			qDebug() << query.lastError();
	}
}

QList<ItemData *> Database::loadData(void)
{
	QList<ItemData *> list;
	QSqlQuery query(this->m_db);

	query.prepare("select * from item;");
	query.exec();

	while (query.next()) {
		ItemData *itemData = new ItemData;
		itemData->md5 = query.value("md5").toByteArray();
		itemData->image = QImage::fromData(query.value("imagedata").toByteArray());
		itemData->time = QDateTime::fromSecsSinceEpoch(query.value("time").toUInt());

		qDebug() << itemData->md5 << itemData->time;

		itemData->mimeData = new QMimeData;
		QSqlQuery query_data(this->m_db);
		query_data.prepare("select * from data where md5 = x'" + itemData->md5.toHex()+"'");
		query_data.exec();

		while (query_data.next()) {
			QString mimeType = query_data.value("formats").toString();
			QByteArray data = query_data.value("format_data").toByteArray();
			qDebug() << mimeType << data;
			itemData->mimeData->setData(mimeType, data);
		}

		list.push_back(itemData);
	}

	return list;
}
