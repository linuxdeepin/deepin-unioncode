// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGPROPERTYDIALOG_H
#define CONFIGPROPERTYDIALOG_H

#include "services/project/projectinfo.h"

#include <QDialog>

class ConfigPropertyWidgetPrivate;
class ConfigPropertyWidget : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigPropertyWidget(const QString &language,
                                  const QString &workspace,
                                  QDialog *parent = nullptr);
    virtual ~ConfigPropertyWidget();

signals:
    void configureDone(const dpfservice::ProjectInfo &info);

private:
    void closeWidget();
    ConfigPropertyWidgetPrivate *const d;
};

#endif // CONFIGPROPERTYDIALOG_H
