// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FINDTOOLWINDOW_H
#define FINDTOOLWINDOW_H

#include "searchresultwindow.h"

#include <QWidget>

class FindToolWindowPrivate;
class FindToolWindow : public QWidget
{
    Q_OBJECT
public:
    explicit FindToolWindow(QWidget *parent = nullptr);
    ~FindToolWindow() override;

signals:

private:
    void setupUi();
    void search();
    void searchText();
    void replace();
    void addSearchParamWidget(QWidget *parentWidget);
    void addSearchResultWidget(QWidget *parentWidget);
    void switchSearchParamWidget();
    bool checkSelectedScopeValid();
    bool getSearchParams(SearchParams *searchParams);
    void createMessageDialog(const QString &message);

    FindToolWindowPrivate *const d;

private slots:
    void onSenseCheckBtnClicked();
    void onwholeWordsCheckBtnClicked();
};

#endif // FINDTOOLWINDOW_H
