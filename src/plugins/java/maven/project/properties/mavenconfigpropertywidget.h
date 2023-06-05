/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
 *
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
#ifndef MAVENCONFIGPROPERTYWIDGET_H
#define MAVENCONFIGPROPERTYWIDGET_H

#include "maven/project/properties/mavenconfigutil.h"

#include "services/project/projectinfo.h"

class PageWidget;
class MavenDetailPropertyWidgetPrivate;
class MavenDetailPropertyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MavenDetailPropertyWidget(QWidget *parent = nullptr);
    ~MavenDetailPropertyWidget();

    void setValues(const mavenConfig::ConfigureParam *param);
    void getValues(mavenConfig::ConfigureParam *param);

public slots:
    void browserFileDialog();

private:
    void setupUI();
    void initData();

    MavenDetailPropertyWidgetPrivate *const d;
};

class MavenConfigPropertyWidgetPrivate;
class MavenConfigPropertyWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit MavenConfigPropertyWidget(const dpfservice::ProjectInfo &projectInfo, QStandardItem *item, QWidget *parent = nullptr);
    virtual ~MavenConfigPropertyWidget() override;

    void saveConfig() override;

public slots:

private:
    void setupUI();
    void initData(const dpfservice::ProjectInfo &projectInfo);

    MavenConfigPropertyWidgetPrivate *const d;
};

#endif // MAVENCONFIGPROPERTYWIDGET_H
