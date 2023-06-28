// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
class PageWidget;

class OptionsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit OptionsDialog(QWidget *parent = nullptr);
    bool insertOptionPanel(const QString &itemName, PageWidget *panel);

public slots:
    void slotLeftBarClicked(const QModelIndex &index);
    void saveSingleConfig();
    void saveAllConfig();

private:
    void setupUi(QDialog *Dialog);
    void readConfig();
    void findOption(const QString &filter);

protected:
    virtual void showEvent(QShowEvent *e) override;
    QLineEdit *filterEdit = nullptr;
    QListView *leftSideBar = nullptr;
    QLabel *headTitle = nullptr;
    QString activeOptName {};
    QMap<QString, PageWidget *> widgts{};
    QStringListModel *leftBarModel = nullptr;
    QStackedWidget *stackWidget = nullptr;
};

#endif // OPTIONSDIALOG_H
