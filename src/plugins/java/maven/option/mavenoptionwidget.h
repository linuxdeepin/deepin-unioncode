// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAVENOPTIONWIDGET_H
#define MAVENOPTIONWIDGET_H

#include "common/common.h"

class MavenOptionWidgetPrivate;
class MavenOptionWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit MavenOptionWidget(QWidget *parent = nullptr);
    ~MavenOptionWidget() override;

    void saveConfig() override;
    void readConfig() override;

signals:

public slots:
private:
    MavenOptionWidgetPrivate *const d;
};

#endif // MAVENOPTIONWIDGET_H
