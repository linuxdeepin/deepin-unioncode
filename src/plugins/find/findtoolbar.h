// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FINDTOOLBAR_H
#define FINDTOOLBAR_H

#include <QWidget>

class CurrentDocumentFind;
class FindToolBarPrivate;
class FindToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit FindToolBar(QWidget *parent = nullptr);

signals:
    void advanced();

private:
    void setupUi();
    void findPrevious();
    void findNext();
    void advancedSearch();
    void replace();
    void replaceSearch();
    void replaceAll();

    FindToolBarPrivate *const d;

};

#endif // FINDTOOLBAR_H
