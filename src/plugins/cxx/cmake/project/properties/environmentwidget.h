// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENVIRONMENTWIDGET_H
#define ENVIRONMENTWIDGET_H

#include "configutil.h"
#include <DFrame>

#include <QAbstractTableModel>

enum EnvType {
    BuildCfg,
    RunCfg
};

class EnvironmentWidgetPrivate;
class EnvironmentWidget : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT

public:
    explicit EnvironmentWidget(QWidget *parent = nullptr, EnvType type = EnvType::BuildCfg);
    virtual ~EnvironmentWidget();

    void appendRow();
    void deleteRow();
    void initModel();

    void getValues(config::EnvironmentItem &env);
    void setValues(const config::EnvironmentItem &env);
    void updateEnvList(config::EnvironmentItem *env);

public slots:
    void handleFocusIndex(const QModelIndex &index);

signals:
    void envUpdated();

private:
    EnvironmentWidgetPrivate *const d;
};

#endif // ENVIRONMENTWIDGET_H
