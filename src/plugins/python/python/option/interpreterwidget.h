// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INTERPRETERWIDGET_H
#define INTERPRETERWIDGET_H

#include "services/option/toolchaindata.h"
#include "common/widget/pagewidget.h"

#include <QAbstractTableModel>

enum ColumnID
{
    kPackage,
    kVersion,
    _KCount
};

struct InterpreterConfig{
    ToolChainData::ToolChainParam version;
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

    void setCustomData(QVector<QPair<QString, QString>>& data);

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

signals:

public slots:

private:
    void setupUi();
    void updateUi();

    bool dataToMap(QMap<QString, QVariant> &map);
    bool mapToData(const QMap<QString, QVariant> &map);

    bool getControlValue(QMap<QString, QVariant> &map);
    void setControlValue(const QMap<QString, QVariant> &map);

    void updatePackageData();
    void findPackages(const QString &cmd);

    InterpreterWidgetPrivate *const d;
};

#endif // INTERPRETERWIDGET_H
