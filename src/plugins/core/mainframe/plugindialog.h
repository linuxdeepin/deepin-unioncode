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

private:
    void updateRestartRequired();
    void updateButtons();
    void openDetails();
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
    QLabel *urlLink = nullptr;
    QTextEdit *description = nullptr;
    QTextEdit *license = nullptr;
    QListWidget *dependencies = nullptr;

    QLabel *label_1 = nullptr;
    QLabel *label_2 = nullptr;
    QLabel *label_3 = nullptr;
    QLabel *label_4 = nullptr;
    QLabel *label_5 = nullptr;
    QLabel *label_6 = nullptr;
    QLabel *label_7 = nullptr;
    QLabel *label_8 = nullptr;
    QLabel *label_9 = nullptr;
    QLabel *label_10 = nullptr;
};

#endif // PLUGINDIALOG_H
