/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef HISTORYVIEW_H
#define HISTORYVIEW_H

#include "basetype.h"

#include <QTableView>
class QStandardItem;
class HistoryViewPrivate;
class HistoryView : public QTableView
{
    Q_OBJECT
    HistoryViewPrivate *const d;

public:
    explicit HistoryView(QWidget *parent = nullptr);
    HistoryData topData() const;
    void setDatas(const HistoryDatas &datas);
    void insertTopDatas(const HistoryDatas &datas);
    void addData(const HistoryData &one);
    QString description(int row) const;
    RevisionFiles revisionFiles(int row) const;
    HistoryData data(int row) const;

private:
    QList<QStandardItem*> createRow(const HistoryData &data) const;
    HistoryData createData(int row) const;
};

#endif // HISTORYVIEW_H
