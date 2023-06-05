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
#ifndef GRADLECONFIGPROPERTYWIDGET_H
#define GRADLECONFIGPROPERTYWIDGET_H

#include "gradle/project/properties/gradleconfigutil.h"
#include "services/project/projectinfo.h"

class PageWidget;
class GradleDetailPropertyWidgetPrivate;
class GradleDetailPropertyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GradleDetailPropertyWidget(QWidget *parent = nullptr);
    ~GradleDetailPropertyWidget();

    void setValues(const gradleConfig::ConfigureParam *param);
    void getValues(gradleConfig::ConfigureParam *param);

public slots:
    void browserFileDialog();

private:
    void setupUI();
    void initData();

    GradleDetailPropertyWidgetPrivate *const d;
};

class GradleConfigPropertyWidgetPrivate;
class GradleConfigPropertyWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit GradleConfigPropertyWidget(const dpfservice::ProjectInfo &projectInfo, QStandardItem *item, QWidget *parent = nullptr);
    virtual ~GradleConfigPropertyWidget() override;

    void saveConfig() override;

private:
    void setupUI();
    void initData(const dpfservice::ProjectInfo &projectInfo);

    GradleConfigPropertyWidgetPrivate *const d;
};

#endif // GRADLECONFIGPROPERTYWIDGET_H
