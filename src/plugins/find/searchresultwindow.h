/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#ifndef SEARCHRESULTWINDOW_H
#define SEARCHRESULTWINDOW_H

#include <QWidget>
#include <QTreeView>

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
class SearchResultTreeView : public QTreeView
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
