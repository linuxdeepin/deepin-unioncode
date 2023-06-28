// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAVENWIDGET_H
#define MAVENWIDGET_H

#include "services/option/toolchaindata.h"
#include "common/widget/pagewidget.h"

struct MavenConfig{
    ToolChainData::ToolChainParam version;
    QString userSetting;
    QString localSetting;
};

class ToolChainData;
class MavenWidgetPrivate;
class MavenWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit MavenWidget(QWidget *parent = nullptr);
    ~MavenWidget() override;

    void setUserConfig(const QMap<QString, QVariant> &map) override;
    void getUserConfig(QMap<QString, QVariant> &map) override;

signals:

public slots:

private:
    void setupUi();
    void updateUi();

    bool dataToMap(const MavenConfig &config, QMap<QString, QVariant> &map);
    bool mapToData(const QMap<QString, QVariant> &map, MavenConfig &config);

    bool getControlValue(QMap<QString, QVariant> &map);
    void setControlValue(const QMap<QString, QVariant> &map);

    MavenWidgetPrivate *const d;
};

#endif // MAVENWIDGET_H
