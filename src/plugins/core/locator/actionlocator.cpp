// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionlocator.h"
#include "common/actionmanager/actionmanager.h"

#include <QAction>
#include <QRegularExpression>

ActionLocator::ActionLocator(QObject *parent)
    : abstractLocator(parent)
{
    setDisplayName("s");
    setDescription(tr("show actions in application"));
    setIncludedDefault(false);
}

void ActionLocator::prepareSearch(const QString &searchText)
{
    Q_UNUSED(searchText)

    locatorList.clear();
    commandList = ActionManager::instance()->commandList();
    foreach (auto command, commandList) {
        if (command->action() && command->action()->isSeparator())
            continue;

        baseLocatorItem item(this);
        item.id = command->id();
        item.displayName = command->description();
        item.extraInfo = Command::keySequencesToNativeString(command->keySequences()).join(" | ");

        locatorList.append(item);
    }
}

QList<baseLocatorItem> ActionLocator::matchesFor(const QString &inputText)
{
    QList<baseLocatorItem> matchResult;
    auto regexp = createRegExp(inputText);

    foreach (auto item, locatorList) {
        auto match = regexp.match(item.displayName);
        if (match.hasMatch())
            matchResult.append(item);
    }

    return matchResult;
}

void ActionLocator::accept(baseLocatorItem item)
{
    foreach (auto command, commandList) {
        if (command->id() == item.id)
            command->action()->trigger();
    }
}
