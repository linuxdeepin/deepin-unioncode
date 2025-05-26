// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <DLabel>
#include <DDialog>
#include <DLineEdit>
#include <DListView>
#include <DTabWidget>
#include <DPushButton>
#include <DStackedWidget>
#include <DHeaderView>
#include <DAbstractDialog>
#include <QMap>

DWIDGET_USE_NAMESPACE

class QHBoxLayout;
class QVBoxLayout;
class QSpacerItem;
class QStringListModel;
class PageWidget;

class OptionsDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit OptionsDialog(QWidget *parent = nullptr);
    bool insertOptionPanel(const QString &itemName, PageWidget *panel);
    bool insertLabel(const QString &itemName);
    void showAtItem(const QString &itemName);

public slots:
    void slotLeftBarClicked(const QModelIndex &index);
    void saveAllConfig();
    void slotScrollChanged(int value);

private:
    void setupUi();
    void readConfig();

protected:
    virtual void showEvent(QShowEvent *e) override;
    DListView *leftSideBar = nullptr;
    QMap<QString, PageWidget *> widgets{};
    QMap<QStandardItem *, DLabel *> itemList{};
    QModelIndex currentIndex;
    QStandardItemModel *leftBarModel = nullptr;
    QScrollArea *scrollArea = nullptr;
    QWidget *scrollWidget = nullptr;
    QVBoxLayout *content = nullptr;
};

#endif // OPTIONSDIALOG_H
