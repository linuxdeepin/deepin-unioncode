// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINWIDGET_H
#define PLUGINWIDGET_H

#include <DTableView>
#include <common/widget/pagewidget.h>

class PluginWidgetPrivate;
class PluginWidget : public PageWidget
{
    Q_OBJECT

public:
    explicit PluginWidget(QWidget *parent = nullptr);
    virtual ~PluginWidget();

    void saveConfig() override;

private slots:
    void updateRestartRequired();

private:
    PluginWidgetPrivate *const d;
};

#endif // PLUGINWIDGET_H
