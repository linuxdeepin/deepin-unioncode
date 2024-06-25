// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FIELDSPANE_H
#define FIELDSPANE_H

#include "abstractpane.h"
#include "templateparser.h"

#include <DWidget>

using DTK_WIDGET_NAMESPACE::DWidget;
class FieldsPane : public AbstractPane
{
    Q_OBJECT
public:
    explicit FieldsPane(const templateMgr::Page &pageInfo, DWidget *parent = nullptr);
    QMap<QString, QVariant> getValue() override;

private:
    void setupUi() override;

    templateMgr::Page page;
    QMap<QString, QVariant> value;
};

#endif   // FIELDSPANE_H
