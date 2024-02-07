// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include "framework/lifecycle/pluginmetaobject.h"

#include <DAbstractDialog>
#include <DPushButton>
#include <DDialogButtonBox>

#include <QSpacerItem>

class DetailsView;
class QDialogButtonBox;
class QLabel;
class PluginStoreWidget;

class PluginsUi : public QObject
{
    Q_OBJECT

public:
    explicit PluginsUi(QObject *parent = nullptr);
    ~PluginsUi();
    PluginStoreWidget *getStoreWidget() const;
    DetailsView *getPluginDetailView() const;

private slots:
    void slotPluginItemSelected();

private:
    void reLaunchRequired();
    void updateButtons();
    void saveConfig();

    PluginStoreWidget *pluginStoreWidget = nullptr;
    DetailsView *pluginDetailView = nullptr;
};



#endif // PLUGINDIALOG_H
