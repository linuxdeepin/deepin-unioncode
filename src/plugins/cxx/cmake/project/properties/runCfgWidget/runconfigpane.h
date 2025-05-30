// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RUNCONFIGPANE_H
#define RUNCONFIGPANE_H

#include "configutil.h"

#include <DWidget>

class RunConfigPanePrivate;
class RunConfigPane : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit RunConfigPane(QWidget *parent = nullptr);
    ~RunConfigPane();

    void setTargetRunParam(config::TargetRunConfigure *targetRunParam);
    void insertTitle(DWidget* lWidget, DWidget *rWidget);

signals:

public slots:

private:
    void setupUi();
    void updateUi();

    RunConfigPanePrivate *const d;
};

#endif // RUNCONFIGPANE_H
