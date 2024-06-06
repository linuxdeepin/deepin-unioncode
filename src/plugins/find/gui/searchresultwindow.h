// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHRESULTWINDOW_H
#define SEARCHRESULTWINDOW_H

#include "constants.h"

#include <QWidget>

class SearchResultWindowPrivate;
class SearchResultWindow : public QWidget
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
    QStringList resultFileList() const;

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
