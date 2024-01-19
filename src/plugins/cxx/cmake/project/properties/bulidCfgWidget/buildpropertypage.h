// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
class BuildPropertyPage : public PageWidget
{
    Q_OBJECT
public:
    explicit BuildPropertyPage(const dpfservice::ProjectInfo &projectInfo, QWidget *parent = nullptr);
    virtual ~BuildPropertyPage() override;

    void readConfig() override;
    void saveConfig() override;

public slots:
    void updateDetail();

private:
    void setupOverviewUI();
    void initData(const dpfservice::ProjectInfo &projectInfo);
    void initRunConfig(const QString &workDirectory, config::RunConfigure &runConfigure);

    BuildPropertyWidgetPrivate *const d;
};

#endif // BUILDPROPERTYWIDGET_H
