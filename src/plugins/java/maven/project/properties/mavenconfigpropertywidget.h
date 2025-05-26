// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAVENCONFIGPROPERTYWIDGET_H
#define MAVENCONFIGPROPERTYWIDGET_H

#include "maven/project/properties/mavenconfigutil.h"

#include "common/project/projectinfo.h"

#include <DFrame>

class PageWidget;
class MavenDetailPropertyWidgetPrivate;
class MavenDetailPropertyWidget : public DTK_WIDGET_NAMESPACE::DFrame
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
