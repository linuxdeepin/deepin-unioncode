// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RUNPROPERTYWIDGET_H
#define RUNPROPERTYWIDGET_H

#include "configutil.h"
#include "common/widget/pagewidget.h"

class QStandardItem;
class RunPropertyWidgetPrivate;
class RunPropertyWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit RunPropertyWidget(const dpfservice::ProjectInfo &projectInfo, QStandardItem *item, QWidget *parent = nullptr);
    virtual ~RunPropertyWidget() override;

    void saveConfig() override;
    void readConfig() override;

private:
    void setupUi();
    void updateData();

    RunPropertyWidgetPrivate *const d;
};

#endif // RUNPROPERTYWIDGET_H
