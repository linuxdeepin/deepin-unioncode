// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHRESULTWINDOW_H
#define SEARCHRESULTWINDOW_H

#include "constants.h"

#include <QWidget>
#include <DTreeView>
#include <DFileIconProvider>

DWIDGET_USE_NAMESPACE

class SearchResultModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SearchResultModel(QObject *parent = nullptr);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void clear();
    FindItem *findItem(const QModelIndex &index) const;
    QString findGroup(const QModelIndex &index) const;
    void appendResult(const FindItemList &list);

private:
    void addGroup(const QString &group);
    void addItem(const QString &group, const FindItemList &itemList);
    QVariant data(const FindItem &item, int role = Qt::DisplayRole) const;
    QVariant data(const QString &group, int role = Qt::DisplayRole) const;

    QMap<QString, FindItemList> resultData;
};

class SearchResultTreeViewPrivate;
class SearchResultTreeView : public DTreeView
{
    Q_OBJECT
    DFileIconProvider iconProvider;

public:
    explicit SearchResultTreeView(QWidget *parent = nullptr);
    ~SearchResultTreeView();

    void appendData(const FindItemList &itemList);
    void clearData();
    virtual QIcon icon(const QString &data);

private:
    SearchResultTreeViewPrivate *const d;
};

class SearchResultWindowPrivate;
class SearchResultWindow : public DWidget
{
    Q_OBJECT
public:
    explicit SearchResultWindow(QWidget *parent = nullptr);
    ~SearchResultWindow();

    void clear();
    void appendResults(const FindItemList &itemList);
    void searchFinished();
    void replaceFinished(bool success);
    void setRepalceWidgtVisible(bool hide);
    void showMsg(bool succeed, QString msg);

signals:
    void reqBack();
    void reqReplace(const QString &text);

private:
    void setupUi();
    void clean();
    void replace();

    SearchResultWindowPrivate *const d;
};

#endif   // SEARCHRESULTWINDOW_H
