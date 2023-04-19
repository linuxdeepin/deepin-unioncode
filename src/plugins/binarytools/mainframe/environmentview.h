/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
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
