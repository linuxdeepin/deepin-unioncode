// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTPANE_H
#define PROJECTPANE_H

#include "abstractpane.h"

#include <DWidget>
#include <DLineEdit>

using DTK_WIDGET_NAMESPACE::DWidget;
class ProjectPane : public AbstractPane
{
    Q_OBJECT
public:
    explicit ProjectPane(const QString &title, DWidget *parent = nullptr);
    QMap<QString, QVariant> getValue() override;

private:
    void setupUi() override;

    QString shortTitle;
    Dtk::Widget::DLineEdit *projectEdit { nullptr };
    Dtk::Widget::DLineEdit *locationEdit { nullptr };
    QMap<QString, QVariant> value;
};

#endif   // PROJECTPANE_H
