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
    };

    QVector<Item> itemVector;
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

bool ToolBarManager::addActionItem(const QString &id, QAction *action)
{
    if (!action || id.isEmpty() || !d->toolbar)
        return false;

    bool exist = false;

    for (int i = 0; i < d->itemVector.size(); i++) {
        if (d->itemVector.at(i).id == id) {
            d->toolbar->removeAction(d->itemVector.at(i).action);
            if (i - 1 >= 0) {
                QAction *before = d->itemVector.at(i - 1).action;
                d->toolbar->insertAction(before, action);
            } else {
                d->toolbar->addAction(action);
            }
            d->itemVector.replace(i, {id, action});
            exist = true;
            break;
        }
    }

    if (!exist) {
        d->toolbar->addAction(action);
        d->itemVector.push_back({id,action});
    }

    return true;
}

bool ToolBarManager::addWidgetItem(const QString &id, QWidget *widget)
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
            d->itemVector.replace(i, {id, a});
            exist = true;
            break;
        }
    }

    if (!exist) {
        a = d->toolbar->addWidget(widget);
        d->itemVector.push_back({id,a});
    }

    return true;
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

