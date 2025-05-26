// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HISTORYDISPLAYWIDGET_H
#define HISTORYDISPLAYWIDGET_H

#include <QSplitter>

class HistoryLogWidget;
class HistoryDiffWidget;
class HistoryDisplayWidget : public QSplitter
{
    Q_OBJECT
public:
    explicit HistoryDisplayWidget(QWidget *parent = nullptr);
    HistoryLogWidget *logWidget();
    HistoryDiffWidget *diffWidget();

private:
    HistoryLogWidget *hisLogWidget{nullptr};
    HistoryDiffWidget *hisDiffWidget{nullptr};
};

#endif // HISTORYDISPLAYWIDGET_H
