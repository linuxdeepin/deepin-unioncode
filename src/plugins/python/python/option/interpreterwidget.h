// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INTERPRETERWIDGET_H
#define INTERPRETERWIDGET_H

#include "services/option/toolchaindata.h"
#include "common/widget/pagewidget.h"

#include <QAbstractTableModel>

enum ColumnID {
    kPackage,
    kVersion,
    _KCount
};

struct InterpreterConfig
{
    ToolChainData::ToolChainParam version;
};

struct QueryInfo
{
    QVector<QPair<QString, QString>> packageList;
    QString errMsg;
};

class InterpreterModelPrivate;
class InterpreterModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit InterpreterModel(QObject *parent = nullptr);
    ~InterpreterModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void setCustomData(const QVector<QPair<QString, QString>> &data);

signals:

private:
    InterpreterModelPrivate *d;
};

class ToolChainData;
class InterpreterWidgetPrivate;
class InterpreterWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit InterpreterWidget(QWidget *parent = nullptr);
    ~InterpreterWidget() override;

    void setUserConfig(const QMap<QString, QVariant> &map) override;
    void getUserConfig(QMap<QString, QVariant> &map) override;

private:
    void setupUi();
    void updateUi();

    bool dataToMap(QMap<QString, QVariant> &map);
    bool mapToData(const QMap<QString, QVariant> &map);

    bool getControlValue(QMap<QString, QVariant> &map);
    void setControlValue(const QMap<QString, QVariant> &map);

    void updatePackageData();
    void queryPackages(const QString &cmd);
    Q_INVOKABLE void applyQueryInfo(const QueryInfo &info);

    InterpreterWidgetPrivate *const d;
};

Q_DECLARE_METATYPE(QueryInfo)

#endif   // INTERPRETERWIDGET_H
