// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRADLEWIDGET_H
#define GRADLEWIDGET_H

#include "services/option/toolchaindata.h"
#include "common/widget/pagewidget.h"

struct GradleConfig{
    ToolChainData::ToolChainParam version;
    QString useWrapper;
    QString useLocal;
};

class ToolChainData;
class GradleWidgetPrivate;
class GradleWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit GradleWidget(QWidget *parent = nullptr);
    ~GradleWidget() override;

    void setUserConfig(const QMap<QString, QVariant> &map) override;
    void getUserConfig(QMap<QString, QVariant> &map) override;

signals:

public slots:

private:
    void setupUi();
    void updateUi();

    bool dataToMap(const GradleConfig &config, QMap<QString, QVariant> &map);
    bool mapToData(const QMap<QString, QVariant> &map, GradleConfig &config);

    bool getControlValue(QMap<QString, QVariant> &map);
    void setControlValue(const QMap<QString, QVariant> &map);

    GradleWidgetPrivate *const d;
};

#endif // GRADLEWIDGET_H
