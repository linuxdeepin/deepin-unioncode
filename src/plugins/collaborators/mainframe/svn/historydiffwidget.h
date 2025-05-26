// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HISTORYDIFFWIDGET_H
#define HISTORYDIFFWIDGET_H

#include <QSplitter>

class HistoryDiffView;
class HistoryDiffWidget : public QSplitter
{
    Q_OBJECT
public:
    explicit HistoryDiffWidget(QWidget *parent = nullptr);
    HistoryDiffView *getOldView() const;
    HistoryDiffView *getNewView() const;

private:
    HistoryDiffView *oldView{nullptr};
    HistoryDiffView *newView{nullptr};
};

#endif // HISTORYDIFFWIDGET_H
