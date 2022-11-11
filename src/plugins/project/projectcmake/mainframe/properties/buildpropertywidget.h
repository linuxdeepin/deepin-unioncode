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
#ifndef BUILDPROPERTYWIDGET_H
#define BUILDPROPERTYWIDGET_H

#include "configutil.h"

#include "common/widget/pagewidget.h"
#include "common/widget/configurewidget.h"

class DetailPropertyWidgetPrivate;
class DetailPropertyWidget : public ConfigureWidget
{
    Q_OBJECT
public:
    explicit DetailPropertyWidget(QWidget *parent = nullptr);
    ~DetailPropertyWidget();

    void setValues(const config::BuildConfigure &configure);
    void getValues(config::BuildConfigure &configure);

private:
    DetailPropertyWidgetPrivate *const d;
};

class BuildPropertyWidgetPrivate;
class BuildPropertyWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit BuildPropertyWidget(QWidget *parent = nullptr);
    virtual ~BuildPropertyWidget() override;

    void readConfig() override;
    void saveConfig() override;

public slots:
    void updateDetail();

private:
    void setupOverviewUI();
    void initData();
    void initRunConfig(const QString &workDirectory, config::RunConfigure &runConfigure);

    BuildPropertyWidgetPrivate *const d;
};

#endif // BUILDPROPERTYWIDGET_H
