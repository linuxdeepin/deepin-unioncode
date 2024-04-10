// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FINDTOOLBAR_H
#define FINDTOOLBAR_H

#include <DFloatingWidget>

class FindToolBarPlaceHolder;
class FindToolBarPrivate;
class FindToolBar : public DTK_WIDGET_NAMESPACE::DFloatingWidget
{
    Q_OBJECT
public:
    explicit FindToolBar(QWidget *parent = nullptr);
    ~FindToolBar();

    void openFindToolBar();

public Q_SLOTS:
    void findNext();
    void findPrevious();
    void replace();
    void replaceFind();
    void replaceAll();
    void findTextChanged();
    void hideAndResetFocus();

private:
    void setFindText(const QString &text);
    static FindToolBarPlaceHolder *findToolBarPlaceHolder();

private:
    FindToolBarPrivate *const d;
};

#endif   // FINDTOOLBAR_H
