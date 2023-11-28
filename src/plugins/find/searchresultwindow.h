// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHRESULTWINDOW_H
#define SEARCHRESULTWINDOW_H

#include <QWidget>
#include <DTreeView>

DWIDGET_USE_NAMESPACE
typedef struct {
    QStringList filePathList;
    QString searchText;
    bool sensitiveFlag;
    bool wholeWordsFlag;
    QStringList patternsList;
    QStringList exPatternsList;
    QMap<QString, QString> projectInfoMap;
}SearchParams;

typedef struct{
    QString filePathName;
    int lineNumber;
    QString context;
}FindItem;

using FindItemList = QList<FindItem>;

class SearchResultTreeViewPrivate;
class SearchResultTreeView : public DTreeView
{
    Q_OBJECT
public:
    explicit SearchResultTreeView(QWidget *parent = nullptr);
    void setData(FindItemList &itemList, QMap<QString, QString> projectInfoMap);
    void clearData();

signals:
private:
    SearchResultTreeViewPrivate *const d;
};

class SearchResultWindowPrivate;
class SearchResultWindow : public QWidget
{
    Q_OBJECT
public:
    explicit SearchResultWindow(QWidget *parent = nullptr);
    void search(SearchParams *params);
    void setRepalceWidgtVisible(bool hide);
    void startSearch(const QString &cmd, const QString &filePath, QMap<QString, QString> projectInfoMap);
    void startReplace(const QStringList &options);
    void searchAgain();
    void showMsg(bool succeed, QString msg);

signals:
    void back();

private:
    void setupUi();
    void clean();
    void replace();

    SearchResultWindowPrivate *const d;
};

#endif // SEARCHRESULTWINDOW_H
