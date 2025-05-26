// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NINJAWIDGET_H
#define NINJAWIDGET_H

#include "services/option/toolchaindata.h"
#include "common/widget/pagewidget.h"

struct NinjaConfig{
    ToolChainData::ToolChainParam version;
};

class ToolChainData;
class NinjaWidgetPrivate;
class NinjaWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit NinjaWidget(QWidget *parent = nullptr);
    ~NinjaWidget() override;

    void setUserConfig(const QMap<QString, QVariant> &map) override;
    void getUserConfig(QMap<QString, QVariant> &map) override;

signals:

public slots:

private:
    void setupUi();
    void updateUi();

    bool dataToMap(const NinjaConfig &config, QMap<QString, QVariant> &map);
    bool mapToData(const QMap<QString, QVariant> &map, NinjaConfig &config);

    bool getControlValue(QMap<QString, QVariant> &map);
    void setControlValue(const QMap<QString, QVariant> &map);

    NinjaWidgetPrivate *const d;
};

#endif // NINJAWIDGET_H
