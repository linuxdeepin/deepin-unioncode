// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstracteditormenu.h"

AbstractEditorMenu::AbstractEditorMenu(QObject *parent)
    : QObject(parent)
{
}

AbstractEditorMenu::~AbstractEditorMenu()
{
}

bool AbstractEditorMenu::initialize(const QVariantHash &params)
{
    auto current = subMenu;
    for (AbstractEditorMenu *scene : current) {
        if (!scene->initialize(params)) {
            subMenu.removeOne(scene);
            delete scene;
        }
    }

    return true;
}

bool AbstractEditorMenu::create(QMenu *parent)
{
    for (AbstractEditorMenu *scene : subMenu)
        scene->create(parent);
    return true;
}

void AbstractEditorMenu::updateState(QMenu *parent)
{
    for (AbstractEditorMenu *scene : subMenu)
        scene->updateState(parent);
}

bool AbstractEditorMenu::triggered(QAction *action)
{
    for (AbstractEditorMenu *scene : subMenu)
        if (scene->triggered(action))
            return true;

    return false;
}

AbstractEditorMenu *AbstractEditorMenu::menu(QAction *action) const
{
    for (AbstractEditorMenu *scene : subMenu)
        if (auto from = scene->menu(action))
            return from;

    return nullptr;
}

bool AbstractEditorMenu::addSubmenu(AbstractEditorMenu *menu)
{
    if (!menu)
        return false;

    menu->setParent(this);
    subMenu.append(menu);
    return true;
}

void AbstractEditorMenu::removeSubmenu(AbstractEditorMenu *menu)
{
    if (menu && menu->parent() == this)
        menu->setParent(nullptr);

    subMenu.removeOne(menu);
}

void AbstractEditorMenu::setSubmenu(const QList<AbstractEditorMenu *> &menus)
{
    //! if these already were subscenes before setting, the caller is responsible for managing their lifecycle

    subMenu = menus;
    for (auto scene : menus)
        scene->setParent(this);
}
