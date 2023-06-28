// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KITSMANAGERWIDGET_H
#define KITSMANAGERWIDGET_H

#include "services/option/toolchaindata.h"
#include "common/widget/pagewidget.h"

struct KitConfig{
    QString name;
    ToolChainData::ToolChainParam ccompiler;
    ToolChainData::ToolChainParam cppcompiler;
    ToolChainData::ToolChainParam debugger;
    ToolChainData::ToolChainParam cmake;
    QString cmakeGenerator;
};

class KitsManagerWidgetPrivate;
class KitsManagerWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit KitsManagerWidget(QWidget *parent = nullptr);
    ~KitsManagerWidget() override;

    void setUserConfig(const QMap<QString, QVariant> &map) override;
    void getUserConfig(QMap<QString, QVariant> &map) override;

signals:

public slots:

private:
    void setupUi();
    void updateUi();

    bool dataToMap(const KitConfig &config, QMap<QString, QVariant> &map);
    bool mapToData(const QMap<QString, QVariant> &map, KitConfig &config);

    bool getControlValue(QMap<QString, QVariant> &map);
    void setControlValue(const QMap<QString, QVariant> &map);

    KitsManagerWidgetPrivate *const d;
};

#endif // KITSMANAGERWIDGET_H
