/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
class InterperterWidgetPrivate;
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

    bool dataToMap(const InterpreterConfig &config, QMap<QString, QVariant> &map);
    bool mapToData(const QMap<QString, QVariant> &map, InterpreterConfig &config);

    bool getControlValue(QMap<QString, QVariant> &map);
    void setControlValue(const QMap<QString, QVariant> &map);

    void setPackageData(const QString &text);
    void findPackages(const QString &cmd);

    InterperterWidgetPrivate *const d;
};

#endif // INTERPRETERWIDGET_H
