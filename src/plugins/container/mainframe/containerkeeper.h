// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTAINERKEEPER_H
#define CONTAINERKEEPER_H

#include "qtermwidget.h" // 3drparty
#include "virtualtabwidget.h"
#include "ColorScheme.h"

#include <QDir>

class QTabWidget;
class VirtualConsole;
class VirtualTabWidget;
class ContainerKeeper final : public QObject
{
    Q_OBJECT
public:
    static ContainerKeeper *instance();
    QTabWidget *tabWidget();
private:
    QTabWidget *qTabWidget{nullptr};
    explicit ContainerKeeper(QObject *parent = nullptr);
    virtual ~ContainerKeeper();
};

#endif // CONTAINERKEEPER_H
