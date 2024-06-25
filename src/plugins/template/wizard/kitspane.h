// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KITSPANE_H
#define KITSPANE_H

#include "abstractpane.h"
#include "templateparser.h"

#include <DWidget>

using DTK_WIDGET_NAMESPACE::DWidget;
class KitsPane : public AbstractPane
{
    Q_OBJECT
public:
    explicit KitsPane(const templateMgr::Page &pageInfo, DWidget *parent = nullptr);
    QMap<QString, QVariant> getValue() override;

private:
    void setupUi() override;

    templateMgr::Page page;
    QMap<QString, QVariant> value;
};

#endif   // KITSPANE_H
