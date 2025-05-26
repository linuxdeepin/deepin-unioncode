// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILWIDGET_H
#define DETAILWIDGET_H

#include "copilot.h"
#include "services/option/toolchaindata.h"
#include "common/widget/pagewidget.h"

struct ChatConfig{
    Chat::Locale globalLanguage = Chat::Zh;
    Chat::Locale commitsLanguage = Chat::Zh;
};

class DetailWidgetPrivate;
class DetailWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit DetailWidget(QWidget *parent = nullptr);
    ~DetailWidget() override;

    void setUserConfig(const QMap<QString, QVariant> &map) override;
    void getUserConfig(QMap<QString, QVariant> &map) override;

signals:

public slots:

private:
    void setupUi();

    bool dataToMap(const ChatConfig &config, QMap<QString, QVariant> &map);
    bool mapToData(const QMap<QString, QVariant> &map, ChatConfig &config);

    bool getControlValue(QMap<QString, QVariant> &map);
    void setControlValue(const QMap<QString, QVariant> &map);

    DetailWidgetPrivate *const d;
};

#endif // DETAILWIDGET_H
