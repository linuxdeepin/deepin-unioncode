// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHRESULTWIDGET_H
#define SEARCHRESULTWIDGET_H

#include "constants.h"

#include <QWidget>

class SearchResultWidgetPrivate;
class SearchResultWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SearchResultWidget(QWidget *parent = nullptr);
    ~SearchResultWidget();

    void clear();
    void setReplaceText(const QString &text, bool regex);
    void appendResults(const FindItemList &itemList);
    QMap<QString, FindItemList> allResult() const;
    bool isEmpty() const;

    void expandAll();
    void collapseAll();
    void showMessage(const QString &msg, MessageType type = Information);

Q_SIGNALS:
    void requestReplace(const QMap<QString, FindItemList> &resultMap);
    void resultCountChanged();

private:
    SearchResultWidgetPrivate *const d;
};

#endif   // SEARCHRESULTWIDGET_H
