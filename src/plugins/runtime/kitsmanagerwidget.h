/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#ifndef KITSMANAGERWIDGET_H
#define KITSMANAGERWIDGET_H

#include <QWidget>
#include <QSet>
#include <QMap>

class ToolChainData
{
public:
    struct ToolChainParam
    {
        QString name;
        QString path;
    };
    using Params = QVector<ToolChainParam>;

    // ToolChain type & Parameters.
    using ToolChains = QMap<QString, Params>;

    ToolChainData();

    bool readToolChain(QString &filePath);
    const ToolChains &getToolChanins() const;

private:
    ToolChains toolChains;
};

class KitsManagerWidgetPrivate;
class KitsManagerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KitsManagerWidget(QWidget *parent = nullptr);
    ~KitsManagerWidget();

signals:

public slots:

private:
    void setupUi(QWidget *Widget);
    void updateUi();

    QSharedPointer<ToolChainData> toolChainData;
    KitsManagerWidgetPrivate *const d;
};

#endif // KITSMANAGERWIDGET_H
