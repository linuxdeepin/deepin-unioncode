// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <DAbstractDialog>
#include <DDialog>
#include <DLineEdit>
#include <DListView>
#include <DLabel>
#include <DTitlebar>
#include <DStackedWidget>

#include <QtWidgets/QDialog>
#include <QMap>

DWIDGET_USE_NAMESPACE

class QHBoxLayout;
class QVBoxLayout;
class QLineEdit;
class QListView;
class QLabel;
class QSpacerItem;
class QStringListModel;
class QStackedWidget;
class PageWidget;

class PropertiesDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit PropertiesDialog(QWidget *parent = nullptr);

    bool insertPropertyPanel(const QString &itemName, PageWidget *panel);
    void showPropertyPanel(const QString &itemName, const QString &tabName);
    void setCurrentTitle(const QString &title);
signals:

public slots:
    void slotLeftBarClicked(const QModelIndex &index);
    void saveSingleConfig();
    void saveAllConfig();
    void slotFilterText(const QString &text);

private:
    void setupUi(DAbstractDialog *Dialog);
    void readConfig();
    void savePropertiesToFile();
    void readPropertiesToFile();

    DListView *leftSideBar = nullptr;
    DLabel *headTitle = nullptr;

    QMap<QString, PageWidget *> widgts;
    QStringListModel *leftBarModel = nullptr;
    DStackedWidget *stackWidget = nullptr;

    QStringList leftBarValues;
    DTitlebar *titleBar = nullptr;
};

#endif // PROPERTIESDIALOG_H
