// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHRESULTWINDOW_H
#define SEARCHRESULTWINDOW_H

#include "constants.h"

#include <QWidget>
#include <DTreeView>
#include <DFileIconProvider>
#include <QStandardItem>

DWIDGET_USE_NAMESPACE

class ItemProxy : public QObject
{
    Q_OBJECT
public:
    explicit ItemProxy(QObject *parent = nullptr);

    void setRuningState(bool isRuning);

public Q_SLOTS:
    void addTask(const FindItemList &itemList);

Q_SIGNALS:
    void taskCompleted(const QList<QStandardItem *> &itemList);

private:
    QAtomicInteger<bool> isRuning { false };
};

class SearchResultTreeViewPrivate;
class SearchResultTreeView : public DTreeView
{
    Q_OBJECT
    DFileIconProvider iconProvider;

public:
    explicit SearchResultTreeView(QWidget *parent = nullptr);
    ~SearchResultTreeView();

    void appendData(const FindItemList &itemList, const ProjectInfo &projectInfo);
    void clearData();
    virtual QIcon icon(const QString &data);

private Q_SLOTS:
    void appendItems(const QList<QStandardItem *> &itemList);

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
    void appendResults(const FindItemList &itemList, const ProjectInfo &projectInfo);
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
