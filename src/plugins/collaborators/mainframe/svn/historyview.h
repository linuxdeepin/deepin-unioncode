// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
