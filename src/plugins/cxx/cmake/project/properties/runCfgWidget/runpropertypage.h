// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RUNPROPERTYWIDGET_H
#define RUNPROPERTYWIDGET_H

#include "configutil.h"
#include "common/widget/pagewidget.h"

class QStandardItem;
class RunPropertyWidgetPrivate;
class RunPropertyPage : public PageWidget
{
    Q_OBJECT
public:
    explicit RunPropertyPage(const dpfservice::ProjectInfo &projectInfo, QStandardItem *item, QWidget *parent = nullptr);
    virtual ~RunPropertyPage() override;

    void saveConfig() override;
    void readConfig() override;

private:
    void setupUi();
    void updateData();

    RunPropertyWidgetPrivate *const d;
};

#endif // RUNPROPERTYWIDGET_H
