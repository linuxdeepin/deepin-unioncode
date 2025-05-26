// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NAMEVALUEMODEL_H
#define NAMEVALUEMODEL_H

#include <QAbstractTableModel>

class NameValueModelPrivate;
class NameValueModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit NameValueModel(QObject *parent = nullptr);
    ~NameValueModel() override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void setItems(const QVariantMap &items);
    QVariantMap items() const;
    QModelIndex addItem();
    QModelIndex addItem(const QString &name, const QVariant &value);
    void removeItem(const QModelIndex &index);
    void removeItem(const QString &variable);
    void clear();

    QString variableFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromVariable(const QString &name) const;

signals:
    void focusIndex(const QModelIndex &index);

private:
    NameValueModelPrivate *const d;
};

#endif   // NAMEVALUEMODEL_H
