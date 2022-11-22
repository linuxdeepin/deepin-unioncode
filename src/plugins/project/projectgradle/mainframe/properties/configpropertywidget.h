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
#ifndef CONFIGPROPERTYWIDGET_H
#define CONFIGPROPERTYWIDGET_H

#include "mainframe/properties/configutil.h"

#include "services/project/projectinfo.h"

class PageWidget;
class DetailPropertyWidgetPrivate;
class DetailPropertyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DetailPropertyWidget(QWidget *parent = nullptr);
    ~DetailPropertyWidget();

    void setValues(const config::ConfigureParam *param);
    void getValues(config::ConfigureParam *param);

private:
    void setupUI();
    void initData();

    DetailPropertyWidgetPrivate *const d;
};

class ConfigPropertyWidgetPrivate;
class ConfigPropertyWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit ConfigPropertyWidget(const dpfservice::ProjectInfo &projectInfo, QWidget *parent = nullptr);
    virtual ~ConfigPropertyWidget() override;

    void saveConfig() override;

public slots:

private:
    void setupUI();
    void initData(const dpfservice::ProjectInfo &projectInfo);

    ConfigPropertyWidgetPrivate *const d;
};

#endif // CONFIGPROPERTYWIDGET_H
