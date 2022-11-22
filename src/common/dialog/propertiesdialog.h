/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <QtWidgets/QDialog>
#include <QMap>

class QHBoxLayout;
class QVBoxLayout;
class QLineEdit;
class QListView;
class QLabel;
class QSpacerItem;
class QStringListModel;
class QStackedWidget;
class PageWidget;

class PropertiesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PropertiesDialog(QWidget *parent = nullptr);

    bool insertPropertyPanel(const QString &itemName, PageWidget *panel);
    void showPropertyPanel(const QString &itemName, const QString &tabName);
signals:

public slots:
    void slotLeftBarClicked(const QModelIndex &index);
    void saveSingleConfig();
    void saveAllConfig();
    void slotFilterText(const QString &text);

private:
    void setupUi(QDialog *Dialog);
    void readConfig();
    void savePropertiesToFile();
    void readPropertiesToFile();

    QLineEdit *filterEdit = nullptr;
    QListView *leftSideBar = nullptr;
    QLabel *headTitle = nullptr;

    QMap<QString, PageWidget *> widgts;
    QStringListModel *leftBarModel = nullptr;
    QStackedWidget *stackWidget = nullptr;

    QStringList leftBarValues;
};

#endif // PROPERTIESDIALOG_H
