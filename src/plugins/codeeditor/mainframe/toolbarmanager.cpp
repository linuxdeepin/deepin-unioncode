/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
 *
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
#include "toolbarmanager.h"

class ToolBarManagerPrivate
{
    friend class ToolBarManager;
    struct Item {
        QString id;
        QAction *action;
        QString group;
    };

    QMap<QString, QAction *> groupIndex;
    QList<Item> itemVector;
    QToolBar *toolbar = nullptr;
};

ToolBarManager::ToolBarManager(const QString &name, QObject *parent)
    : QObject(parent)
    , d(new ToolBarManagerPrivate())
{
    d->toolbar = new QToolBar(name);
    d->toolbar->setIconSize(QSize(25, 25));
}

ToolBarManager::~ToolBarManager()
{
    if (d) {
        delete d;
    }
}

QToolBar *ToolBarManager::getToolBar() const
{
    return d->toolbar;
}

bool ToolBarManager::addActionItem(const QString &id, QAction *action, const QString &group)
{
    if (!action || id.isEmpty() || !d->toolbar || hasOverrideActionItem(id, action, group))
        return false;
    QStringList groupIndex = group.split(".");
    QString groupName = groupIndex.at(0);

    if(!d->groupIndex.contains(groupName)) {
        d->groupIndex.insert(groupName, nullptr);
    }

    auto currentIterator = d->groupIndex.find(groupName);
    auto nextIterator = currentIterator + 1;
    QAction *before = nullptr;
    for (; nextIterator != d->groupIndex.end(); ++nextIterator) {
        if (nextIterator.value() != nullptr) {
            QString temp = nextIterator.key();
            before = nextIterator.value();
            break;
        }
    }

    d->toolbar->insertAction(before, action);

    if (currentIterator.value() == nullptr)
        d->groupIndex[groupName] = action;
    if (groupIndex.size() > 1) {
        if (groupIndex.at(1) == "Start") {
            if (currentIterator.value() != nullptr && currentIterator.value() != action)
                before = currentIterator.value();
            d->toolbar->insertAction(before, action);
            currentIterator.value() = action;
        } else if (groupIndex.at(1) == "End") {
            d->toolbar->insertSeparator(before);
        }
    }

    return true;
}

bool ToolBarManager::addWidgetItem(const QString &id, QWidget *widget, const QString &group)
{
    if (!widget || id.isEmpty() || !d->toolbar)
        return false;

    bool exist = false;
    QAction *a = nullptr;

    for (int i = 0; i < d->itemVector.size(); i++) {
        if (d->itemVector.at(i).id == id) {
            d->toolbar->removeAction(d->itemVector.at(i).action);
            if (i - 1 >= 0) {
                QAction *before = d->itemVector.at(i - 1).action;
                a = d->toolbar->insertWidget(before, widget);
            } else {
                a = d->toolbar->addWidget(widget);
            }
            d->itemVector.replace(i, {id, a, group});
            exist = true;
            break;
        }
    }

    if (!exist) {
        a = d->toolbar->addWidget(widget);
        d->itemVector.push_back({id, a, group});
    }

    return true;
}

bool ToolBarManager::hasOverrideActionItem(const QString &id, QAction *action, const QString &group)
{
    for (int i = 0; i < d->itemVector.size(); i++) {
        if (d->itemVector.at(i).id == id) {
            d->toolbar->removeAction(d->itemVector.at(i).action);
            if (i - 1 >= 0) {
                QAction *before = d->itemVector.at(i - 1).action;
                d->toolbar->insertAction(before, action);
            } else {
                d->toolbar->addAction(action);
            }
            d->itemVector.replace(i, {id, action, group});
            return true;
        }
    }

    return false;
}

void ToolBarManager::removeItem(const QString &id)
{
    for (auto iter = d->itemVector.begin(); iter != d->itemVector.end(); ++iter) {
        if (iter->id == id) {
            d->toolbar->removeAction(iter->action);
            d->itemVector.erase(iter);
            return;
        }
    }
}

void ToolBarManager::disableItem(const QString &id, bool disable)
{
    for (auto iter = d->itemVector.begin(); iter != d->itemVector.end(); ++iter) {
        if (iter->id == id) {
            iter->action->setDisabled(disable);
            return;
        }
    }
}
