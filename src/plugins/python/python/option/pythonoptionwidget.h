// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PYTHONOPTIONWIDGET_H
#define PYTHONOPTIONWIDGET_H

#include "common/common.h"

class PythonOptionWidgetPrivate;
class PythonOptionWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit PythonOptionWidget(QWidget *parent = nullptr);
    ~PythonOptionWidget() override;

    void saveConfig() override;
    void readConfig() override;

signals:

public slots:
private:
    PythonOptionWidgetPrivate *const d;
};

#endif // PYTHONOPTIONWIDGET_H
