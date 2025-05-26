// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JDKWIDGET_H
#define JDKWIDGET_H

#include "services/option/toolchaindata.h"
#include "common/widget/pagewidget.h"

struct JDKConfig{
    ToolChainData::ToolChainParam version;
};

class ToolChainData;
class JDKWidgetPrivate;
class JDKWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit JDKWidget(QWidget *parent = nullptr);
    ~JDKWidget() override;

    void setUserConfig(const QMap<QString, QVariant> &map) override;
    void getUserConfig(QMap<QString, QVariant> &map) override;

signals:

public slots:

private:
    void setupUi();
    void updateUi();

    bool dataToMap(const JDKConfig &config, QMap<QString, QVariant> &map);
    bool mapToData(const QMap<QString, QVariant> &map, JDKConfig &config);

    bool getControlValue(QMap<QString, QVariant> &map);
    void setControlValue(const QMap<QString, QVariant> &map);

    JDKWidgetPrivate *const d;
};

#endif // JDKWIDGET_H
