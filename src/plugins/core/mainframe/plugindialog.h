// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include "framework/lifecycle/pluginmetaobject.h"

#include <QDialog>
#include <QLabel>
#include <QGridLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QSpacerItem>

class DetailsView;
class QDialogButtonBox;
class QLabel;

namespace dpf {
class PluginView;
}

class PluginDialog : public QDialog
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

    QDialogButtonBox *closeButton = nullptr;
    QPushButton *detailsButton = nullptr;
    QLabel *restratRequired = nullptr;
};

class DetailsView : public QWidget
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
    QLabel *name = nullptr;
    QLabel *version = nullptr;
    QLabel *compatVersion = nullptr;
    QLabel *vendor = nullptr;
    QLabel *copyright = nullptr;
    QLabel *category = nullptr;
    QLabel *urlLink = nullptr;
    QTextEdit *description = nullptr;
    QTextEdit *license = nullptr;
    QListWidget *dependencies = nullptr;

    QLabel *nameLabel = nullptr;
    QLabel *versionLabel = nullptr;
    QLabel *compatVersionLabel = nullptr;
    QLabel *vendorLabel = nullptr;
    QLabel *copyrightLabel = nullptr;
    QLabel *categoryLabel = nullptr;
    QLabel *urlLabel = nullptr;
    QLabel *licenseLabel = nullptr;
    QLabel *descriptionLabel = nullptr;
    QLabel *dependenciesLabel = nullptr;
};

#endif // PLUGINDIALOG_H
