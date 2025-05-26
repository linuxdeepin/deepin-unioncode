// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STEPSPANE_H
#define STEPSPANE_H

#include "configutil.h"

#include <DWidget>
#include <QAbstractTableModel>

class StepsModelPrivate;
class StepsModel : public QAbstractTableModel
{
public:
    enum ColumnType
    {
        kCheckBox,
        kTarget,
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
class StepsPane : public DTK_WIDGET_NAMESPACE::DWidget
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
