// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGPROPERTYDIALOG_H
#define CONFIGPROPERTYDIALOG_H

#include "common/project/projectinfo.h"

#include <DWidget>

class ConfigPropertyWidgetPrivate;
class ConfigPropertyWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit ConfigPropertyWidget(const QString &language,
                                  const QString &workspace,
                                  QWidget *parent = nullptr);
    virtual ~ConfigPropertyWidget();
    void accept();

signals:
    void configureDone(const dpfservice::ProjectInfo &info);

private:
    ConfigPropertyWidgetPrivate *const d;
};

#endif // CONFIGPROPERTYDIALOG_H
