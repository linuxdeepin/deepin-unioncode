/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#ifndef STEPSPANE_H
#define STEPSPANE_H

#include "configutil.h"

#include <QWidget>
#include <QAbstractTableModel>

class StepsModelPrivate;
class StepsModel : public QAbstractTableModel
{
public:
    enum ColumnType
    {
        kCheckBox,
        kTarget,
        kPath,
        kColumnCount
    };

    explicit StepsModel(QObject *parent = nullptr);
    ~StepsModel() override;

    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    void setData(const QMap<QString, bool> &data);    
    QString getSelectedTarget();

private:
    StepsModelPrivate *const d;
};

class StepsPanePrivate;
class StepsPane : public QWidget
{
    Q_OBJECT

public:
    explicit StepsPane( QWidget *parent = nullptr);
    ~StepsPane();

    void setValues(const config::StepItem &item);
    void getValues(config::StepItem &item);

private:
    void setupUi();
    QString getCombinedBuildText();
    void updateSummaryText();

    void toolArgumentsEdited();
    void dataChanged();

    StepsPanePrivate *const d;
};


#endif // STEPSPANE_H
