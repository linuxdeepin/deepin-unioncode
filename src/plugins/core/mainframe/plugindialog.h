// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include "framework/lifecycle/pluginmetaobject.h"

#include <DAbstractDialog>
#include <DLabel>
#include <DPushButton>
#include <DDialogButtonBox>
#include <DTextEdit>
#include <DListWidget>

#include <QGridLayout>
#include <QSpacerItem>

DWIDGET_USE_NAMESPACE

class DetailsView;
class QDialogButtonBox;
class QLabel;

namespace dpf {
class PluginView;
}

class PluginDialog : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT

public:
    explicit PluginDialog(QWidget *parent = nullptr);

private slots:
    void slotCurrentPluginActived();

private:
    void updateRestartRequired();
    void updateButtons();
    void closeDialog();

    dpf::PluginView *view = nullptr;
    DetailsView *detailView = nullptr;

    DDialogButtonBox *closeButton = nullptr;
    DPushButton *detailsButton = nullptr;
    DLabel *restratRequired = nullptr;
};

class DetailsView : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    DetailsView(QWidget *parent = nullptr);

    void update(dpf::PluginMetaObjectPointer plugin);

private:
    QGridLayout *gridLayout = nullptr;
    QVBoxLayout *vboxLayout_1 = nullptr;
    QVBoxLayout *vboxLayout_2 = nullptr;
    QVBoxLayout *vboxLayout_3 = nullptr;
    QSpacerItem *spacerItem_1 = nullptr;
    QSpacerItem *spacerItem_2 = nullptr;
    QSpacerItem *spacerItem_3 = nullptr;
    DLabel *name = nullptr;
    DLabel *version = nullptr;
    DLabel *compatVersion = nullptr;
    DLabel *vendor = nullptr;
    DLabel *copyright = nullptr;
    DLabel *category = nullptr;
    DLabel *urlLink = nullptr;
    DTextEdit *description = nullptr;
    DTextEdit *license = nullptr;
    DListWidget *dependencies = nullptr;

    DLabel *nameLabel = nullptr;
    DLabel *versionLabel = nullptr;
    DLabel *compatVersionLabel = nullptr;
    DLabel *vendorLabel = nullptr;
    DLabel *copyrightLabel = nullptr;
    DLabel *categoryLabel = nullptr;
    DLabel *urlLabel = nullptr;
    DLabel *licenseLabel = nullptr;
    DLabel *descriptionLabel = nullptr;
    DLabel *dependenciesLabel = nullptr;
};

#endif // PLUGINDIALOG_H
