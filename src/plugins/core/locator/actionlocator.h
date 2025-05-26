// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACTIONLOCATOR_H
#define ACTIONLOCATOR_H

#include "base/abstractlocator.h"
#include "common/actionmanager/command.h"

#include <QObject>

class ActionLocator : public abstractLocator
{
    Q_OBJECT
public:
    explicit ActionLocator(QObject *parent = nullptr);

    void prepareSearch(const QString &searchText) override;
    QList<baseLocatorItem> matchesFor(const QString &inputText) override;
    void accept(baseLocatorItem item) override;
private:
    QList<Command *> commandList;
    QList<baseLocatorItem> locatorList;
};

#endif // ACTIONLOCATOR_H
