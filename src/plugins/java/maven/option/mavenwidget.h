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
#ifndef MAVENWIDGET_H
#define MAVENWIDGET_H

#include "services/option/toolchaindata.h"
#include "common/widget/pagewidget.h"

struct MavenConfig{
    ToolChainData::ToolChainParam version;
    QString userSetting;
    QString localSetting;
};

class ToolChainData;
class MavenWidgetPrivate;
class MavenWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit MavenWidget(QWidget *parent = nullptr);
    ~MavenWidget() override;

    void setUserConfig(const QMap<QString, QVariant> &map) override;
    void getUserConfig(QMap<QString, QVariant> &map) override;

signals:

public slots:

private:
    void setupUi();
    void updateUi();

    bool dataToMap(const MavenConfig &config, QMap<QString, QVariant> &map);
    bool mapToData(const QMap<QString, QVariant> &map, MavenConfig &config);

    bool getControlValue(QMap<QString, QVariant> &map);
    void setControlValue(const QMap<QString, QVariant> &map);

    MavenWidgetPrivate *const d;
};

#endif // MAVENWIDGET_H
