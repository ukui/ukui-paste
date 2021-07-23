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

#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QObject>
#include <QMutex>

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
    void deleleAllPasteItem();
    void delelePasteItem(QByteArray);
    bool isDataExist(void);
    QList<ItemData *> loadData(void);
    QList<ItemData *> selectPasteItem(QString);

private:
	static QByteArray convertImage2Array(QImage image);
    QSqlDatabase	m_db;
};

#endif // DATABASE_H
