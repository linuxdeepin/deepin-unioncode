// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REPORTPANE_H
#define REPORTPANE_H

#include <QWidget>

class QTableWidget;
class CodePorting;
class ReportPane : public QWidget
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
    void setTableWidgetStyle(QTableWidget *srcTableWidget, const QStringList &colNames);
    void refreshTableView(QTableWidget *widget, const QList<QStringList> &report);

    QTableWidget *srcTableWidget {nullptr};
    QTableWidget *libTableWidget {nullptr};
    CodePorting *codePorting {nullptr};
};

#endif // REPORTPANE_H
