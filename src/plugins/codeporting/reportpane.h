// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REPORTPANE_H
#define REPORTPANE_H

#include <DFrame>
#include <DListView>
#include <DTableWidget>

DWIDGET_USE_NAMESPACE

class QTableWidget;
class CodePorting;
class ReportPane : public DWidget
{
    Q_OBJECT
public:
    explicit ReportPane(CodePorting *codePorting, QWidget *parent = nullptr);
    void refreshDispaly();

signals:
    void selectedChanged(const QString &filePath, const QString &suggestion, int startLine, int endLiness);

public slots:
    void srcCellSelected(const QModelIndex &index);
    void libCellSelected(int row, int col);
    void onChangeReportList(const QString &listName);

private:
    void initUI();
    void setViewItem(DTreeView *view, const QList<QStringList> &report);

    CodePorting *codePorting {nullptr};

    DTreeView *srcView {nullptr};
    DTreeView *libView {nullptr};
};

#endif // REPORTPANE_H
