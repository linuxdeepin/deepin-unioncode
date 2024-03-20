// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODECOMPLETIONVIEW_H
#define CODECOMPLETIONVIEW_H

#include <QListView>

class CodeCompletionView : public QListView
{
    Q_OBJECT
public:
    explicit CodeCompletionView(QWidget *parent = nullptr);

    void nextCompletion();
    void previousCompletion();
};

#endif // CODECOMPLETIONVIEW_H
