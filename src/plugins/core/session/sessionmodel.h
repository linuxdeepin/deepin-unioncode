// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONMODEL_H
#define SESSIONMODEL_H

#include <QAbstractTableModel>

class SessionModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SessionModel(QObject *parent = nullptr);

    int rowOfSession(const QString &session) const;
    QString sessionAt(int row) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

public Q_SLOTS:
    void reset();

private:
    QStringList sessionList;
    int sortColumn { 0 };
    Qt::SortOrder sortOrder { Qt::AscendingOrder };
};

#endif   // SESSIONMODEL_H
