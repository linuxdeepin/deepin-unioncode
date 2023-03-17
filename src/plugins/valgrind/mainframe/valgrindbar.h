/*
 * Copyright (C) 2020 ~ 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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
#ifndef VALGRINDBAR_H
#define VALGRINDBAR_H

#include <QWidget>

class QTableWidget;
class QTreeWidget;
class ValgrindBarPrivate;
class ValgrindBar : public QWidget
{
    Q_OBJECT
public:
    explicit ValgrindBar(QWidget *parent = nullptr);
    void refreshDisplay(QTreeWidget *treeWidget);

public slots:
    void clearDisplay(const QString &type);

private:
    void initValgrindbar();
    void setWidgetStyle(QTreeWidget *treeWidget, const QStringList &itemNames);
    void showResult(const QString &xmlFileName, const QString &type);

    ValgrindBarPrivate *const d;
};

#endif // VALGRINDBAR_H
