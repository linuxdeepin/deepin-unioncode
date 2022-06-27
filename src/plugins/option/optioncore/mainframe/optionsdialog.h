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
#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

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

class OptionsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit OptionsDialog(QWidget *parent = nullptr);
    bool insertOptionPanel(const QString &itemName, QWidget *panel);

public slots:
    void slotLeftBarClicked(const QModelIndex &index);
    void saveSingleConfig();
    void saveAllConfig();

private:
    void setupUi(QDialog *Dialog);
    void readConfig();

protected:
    virtual void showEvent(QShowEvent *e) override;
    QLineEdit *filterEdit = nullptr;
    QListView *leftSideBar = nullptr;
    QLabel *headTitle = nullptr;
    QString activeOptName {};
    QMap<QString, QWidget *> widgts{};
    QStringListModel *leftBarModel = nullptr;
    QStackedWidget *stackWidget = nullptr;
};

#endif // OPTIONSDIALOG_H
