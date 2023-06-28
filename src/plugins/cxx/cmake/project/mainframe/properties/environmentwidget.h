// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
