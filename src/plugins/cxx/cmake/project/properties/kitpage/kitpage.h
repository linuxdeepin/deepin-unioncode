// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KITPAGE_H
#define KITPAGE_H

#include "common/widget/pagewidget.h"
#include "common/project/projectinfo.h"

#include <DComboBox>

class KitPage : public PageWidget
{
public:
    explicit KitPage(const dpfservice::ProjectInfo &projectInfo, QStandardItem *item, QWidget *parent = nullptr);

    void readConfig() override;
    void saveConfig() override;

private:
    void initUI();

    DTK_WIDGET_NAMESPACE::DComboBox *kitComboBox { nullptr };
    dpfservice::ProjectInfo projectInfo;
    QStandardItem *item { nullptr };
};

#endif   // KITPAGE_H
