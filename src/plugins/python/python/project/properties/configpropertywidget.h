// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGPROPERTYWIDGET_H
#define CONFIGPROPERTYWIDGET_H

#include "python/project/properties/configutil.h"

#include "common/project/projectinfo.h"

class PageWidget;
class DetailPropertyWidgetPrivate;
class DetailPropertyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DetailPropertyWidget(QWidget *parent = nullptr);
    ~DetailPropertyWidget();

    void setValues(const config::ProjectConfigure *param);
    void getValues(config::ProjectConfigure *param);

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
    explicit ConfigPropertyWidget(const dpfservice::ProjectInfo &projectInfo, QStandardItem *item, QWidget *parent = nullptr);
    virtual ~ConfigPropertyWidget() override;

    void saveConfig() override;

public slots:

private:
    void setupUI();
    void initData(const dpfservice::ProjectInfo &projectInfo);

    ConfigPropertyWidgetPrivate *const d;
};

#endif // CONFIGPROPERTYWIDGET_H
