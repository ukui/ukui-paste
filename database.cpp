#include "database.h"

#include <QApplication>
#include <QSqlError>
#include <QBuffer>
#include <QSqlRecord>
#include <QDateTime>
#include <QThread>
#include <QDebug>

#define DEBUG() qDebug()<<__FILE__<<__func__<<__LINE__

Database::Database(QObject *parent) : QObject(parent)
{
	this->m_db = QSqlDatabase::addDatabase("QSQLITE");
	this->m_db.setUserName("root");
	this->m_db.setPassword("QeErTyUiOp{]");
#ifdef Q_OS_LINUX
	this->m_db.setDatabaseName(QString(getenv("HOME")) + "/.cache/PastesDatabase.db");
#endif
#ifdef Q_OS_WIN
	this->m_db.setDatabaseName(QCoreApplication::applicationDirPath() + "/" + "PastesDatabase.db");
#endif
	DEBUG() << this->m_db.databaseName();

	if (!this->m_db.open())
		DEBUG() << this->m_db.lastError();
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

	if (!query.exec("create table if not exists item(id integer primary key autoincrement, md5 blob, imagedata blob, icondata blob, time integer)"))
		DEBUG() << query.lastError();

	if (!query.exec("create table if not exists data(id integer primary key autoincrement, md5 blob, formats text, format_data blob)"))
		DEBUG() << query.lastError();
}

QByteArray Database::convertImage2Array(QImage image)
{
	QByteArray imagedata;
	QBuffer buffer(&imagedata);

	buffer.open(QIODevice::WriteOnly);
	image.save(&buffer, "png");
	buffer.close();

	return imagedata;
}

/* Used for lock database */
QMutex mutex;

void Database::insertPasteItem(ItemData *itemData)
{
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

void Database::delelePasteItem(QByteArray md5)
{
	QThread *delete_thread = QThread::create([this, md5](void){
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

QList<ItemData *> Database::loadData(void)
{
	QList<ItemData *> list;
	QSqlQuery query(this->m_db);

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
