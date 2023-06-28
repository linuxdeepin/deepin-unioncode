// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENVIRONMENTVIEW_H
#define ENVIRONMENTVIEW_H

#include <QAbstractTableModel>
#include <QWidget>

class EnvironmentModelPrivate;
class EnvironmentModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum ColumnType
    {
        Key,
        Value,
        ColumnCount
    };

    explicit EnvironmentModel(QObject *parent = nullptr);
    ~EnvironmentModel() override;

    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void append(const QString &key, const QVariant &value);
    void remove(QModelIndex &index);
    void update(const QMap<QString, QVariant> &data);
    const QMap<QString, QVariant> getEnvironment() const;

private:
    EnvironmentModelPrivate *const d;
};

class EnvironmentViewPrivate;
class EnvironmentView : public QWidget
{
    Q_OBJECT
public:
    explicit EnvironmentView(QWidget *parent = nullptr);
    ~EnvironmentView();

    const QMap<QString, QVariant> getEnvironment();
    void appendRow();
    void deleteRow();
    void initModel();
    void setValue(const QMap<QString, QVariant> &map);

signals:
    void deleteSignal(bool enable);

private:

    EnvironmentViewPrivate *const d;
};

#endif // ENVIRONMENTVIEW_H
