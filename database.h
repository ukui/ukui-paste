#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QObject>

#include "pasteitem.h"

class Database : public QObject
{
	Q_OBJECT
public:
	explicit Database(QObject *parent = nullptr);
	~Database();
	bool isTableExist(void);
	void createTable(void);
	void insertPasteItem(ItemData *itemData);
	QList<ItemData *> loadData(void);

private:
	void deleteTable(void);

private:
	QSqlDatabase	m_db;

};

#endif // DATABASE_H
