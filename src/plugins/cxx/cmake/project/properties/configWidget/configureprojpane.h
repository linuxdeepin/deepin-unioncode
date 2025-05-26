// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGUREPROJPANE_H
#define CONFIGUREPROJPANE_H


#include "configutil.h"

#include "common/project/projectinfo.h"

#include <DWidget>

class ConfigureProjPanePrivate;
class ConfigureProjPane : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    ConfigureProjPane(const QString &language,
                      const QString &workspace,
                      QWidget *parent = nullptr);
    ~ConfigureProjPane();

signals:
    void configureDone(const dpfservice::ProjectInfo &info);

public slots:
    void slotConfigure();

private:
    void setupUI();
    void resetUI();
    void updateUI();
    void configProject(const config::ProjectConfigure *param);

    ConfigureProjPanePrivate *const d;
};

#endif   // CONFIGUREPROJPANE_H
