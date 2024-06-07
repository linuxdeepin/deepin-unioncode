// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FINDTOOLWINDOW_H
#define FINDTOOLWINDOW_H

#include "constants.h"

#include <QWidget>

class FindToolWindowPrivate;
class FindToolWindow : public QWidget
{
    Q_OBJECT
public:
    explicit FindToolWindow(QWidget *parent = nullptr);
    ~FindToolWindow() override;

private:
    void search();
    void searchText();
    void replace();
    void switchSearchParamWidget();
    bool getSearchParams(SearchParams *searchParams);

    friend class FindToolWindowPrivate;
    FindToolWindowPrivate *const d;

private slots:
    void handleSearchMatched();
    void handleSearchFinished();
    void handleReplace(const QString &text);
    void handleReplaceFinished(int result);
};

#endif // FINDTOOLWINDOW_H
