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

#include <QVector>

class ToolBarManagerPrivate
{
    friend class ToolBarManager;
    struct Item {
        QString id;
        QAction *action;
        QString group;
    };

    QList<Item> itemVector;
    QList<QString> actionList;
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

    int index = sortActionGroup(group);

    if (index > 0) {
        index -= 1;
    }

    QAction *before = nullptr;
    if (d->itemVector.size() > index) {
        before = d->itemVector.at(index).action;
    }
    d->toolbar->insertAction(before,action);
    d->itemVector.insert(index, {id, action, group});

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

void ToolBarManager::addSeparator(const QString &group)
{
    int index = sortActionGroup(group);
    if (index >= d->itemVector.size()) {
        d->toolbar->addSeparator();
    } else {
        QAction *before = d->itemVector.at(index - 1).action;
        d->toolbar->insertSeparator(before);
    }
    d->itemVector.insert(index, {group, nullptr, group});
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

int ToolBarManager::sortActionGroup(const QString &group)
{
    int index = 0;
    if (!d->actionList.count(group)) {
        for (int i = 0; i < d->actionList.size(); i++) {
            const QString &temp = d->actionList.at(i);
            if (QString::compare(group, temp) < 0) {
                break;
            } else {
                index++;
            }
        }
    } else {
        index = d->actionList.indexOf(group, 0);
        for (; index < d->actionList.size(); index++) {
            if (d->actionList.at(index) != group) {
                break;
            }
        }
    }
    d->actionList.insert(index, group);
    return index;
}

