// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REPORTPANE_H
#define REPORTPANE_H

#include <DWidget>
#include <DTableWidget>

class QTableWidget;
class CodePorting;
class ReportPane : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit ReportPane(CodePorting *codePorting, QWidget *parent = nullptr);
    void refreshDispaly();

signals:
    void selectedChanged(const QString &filePath, const QString &suggestion, int startLine, int endLiness);

public slots:
    void srcCellSelected(int row, int col);
    void libCellSelected(int row, int col);

private:
    void initTableWidget();
    void setTableWidgetStyle(DTK_WIDGET_NAMESPACE::DTableWidget *srcTableWidget, const QStringList &colNames);
    void refreshTableView(DTK_WIDGET_NAMESPACE::DTableWidget *widget, const QList<QStringList> &report);

    DTK_WIDGET_NAMESPACE::DTableWidget *srcTableWidget {nullptr};
    DTK_WIDGET_NAMESPACE::DTableWidget *libTableWidget {nullptr};
    CodePorting *codePorting {nullptr};
};

#endif // REPORTPANE_H
