/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
#ifndef ENVIRONMENTWIDGET_H
#define ENVIRONMENTWIDGET_H

#include "configutil.h"

#include <QAbstractTableModel>
#include <QWidget>

class EnvironmentModelPrivate;
class EnvironmentModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum ColumnType
    {
        kVaribale,
        kValue,
        kColumnCount
    };

    explicit EnvironmentModel(QObject *parent = nullptr);
    ~EnvironmentModel() override;

    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void append(const QString &key, const QString &value);
    void update(const QMap<QString, QString> &data);
    const QMap<QString, QString> getEnvironment() const;

private:
    EnvironmentModelPrivate *const d;
};

class EnvironmentWidgetPrivate;
class EnvironmentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EnvironmentWidget(QWidget *parent = nullptr);
    virtual ~EnvironmentWidget();

    void getValues(config::EnvironmentItem &env);
    void setValues(const config::EnvironmentItem &env);
    void bindValues(config::EnvironmentItem *env);

private:
    EnvironmentWidgetPrivate *const d;
};

#endif // ENVIRONMENTWIDGET_H
