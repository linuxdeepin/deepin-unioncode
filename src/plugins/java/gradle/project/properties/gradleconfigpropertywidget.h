// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRADLECONFIGPROPERTYWIDGET_H
#define GRADLECONFIGPROPERTYWIDGET_H

#include "gradle/project/properties/gradleconfigutil.h"
#include "common/project/projectinfo.h"
#include <DFrame>

class PageWidget;
class GradleDetailPropertyWidgetPrivate;
class GradleDetailPropertyWidget : public DTK_WIDGET_NAMESPACE::DFrame
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
