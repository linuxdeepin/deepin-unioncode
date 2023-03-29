/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
class QPushButton;
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

    QPushButton *detailsButton = nullptr;
    QPushButton *closeButton = nullptr;
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
