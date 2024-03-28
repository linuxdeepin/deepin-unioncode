// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FINDTOOLBAR_H
#define FINDTOOLBAR_H

#include <QFrame>

class FindToolBarPrivate;
class FindToolBar : public QFrame
{
    Q_OBJECT
public:
    explicit FindToolBar(QWidget *parent = nullptr);
    ~FindToolBar();

    void setFindText(const QString &text);

public slots:
    void invokeFindNext();
    void invokeFindPrevious();
    bool invokeFindStep(bool isForward);
    void invokeReplace();
    void invokeReplaceNext();
    void invokeReplaceAll();
    void onFindTextChanged();

private:
    FindToolBarPrivate *const d;
};

#endif // FINDTOOLBAR_H
