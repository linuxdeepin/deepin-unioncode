// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <DLabel>
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

public slots:
    void slotLeftBarClicked(const QModelIndex &index);
    void saveSingleConfig();
    void saveAllConfig();
    void slotScrollChanged(int value);

private:
    void setupUi(QWidget *widget);
    void readConfig();

protected:
    virtual void showEvent(QShowEvent *e) override;
    DListView *leftSideBar = nullptr;
    QString activeOptName {};
    QMap<QString, PageWidget *> widgets{};
    QStringListModel *leftBarModel = nullptr;
    QScrollArea *scrollArea = nullptr;
    QWidget *scrollWidget = nullptr;
    QVBoxLayout *content = nullptr;
};

#endif // OPTIONSDIALOG_H
