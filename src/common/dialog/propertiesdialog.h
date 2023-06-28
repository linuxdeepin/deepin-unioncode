// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
