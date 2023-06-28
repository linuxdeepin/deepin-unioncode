// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HISTORYLOGWIDGET_H
#define HISTORYLOGWIDGET_H

#include <QSplitter>

class QTextBrowser;
class HistoryView;
class FileModifyView;
class HistoryLogWidget : public QSplitter
{
    Q_OBJECT
public:
    explicit HistoryLogWidget(QWidget *parent = nullptr);
    QTextBrowser *descriptionBrowse();
    HistoryView *historyView();
    FileModifyView *fileChangedView();

private:
    QTextBrowser *descBrowse{nullptr};
    HistoryView *hisView{nullptr};
    FileModifyView *changedView{nullptr};
};

#endif // HISTORYLOGWIDGET_H
