/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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

    QTableWidget *srcTableWidget {nullptr};
    QTableWidget *libTableWidget {nullptr};
    CodePorting *codePorting {nullptr};
};

#endif // REPORTPANE_H
