// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    ~ValgrindBar();

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
