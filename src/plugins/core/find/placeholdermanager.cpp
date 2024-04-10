// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "placeholdermanager.h"
#include "findtoolbar.h"

#include <QVBoxLayout>

FindToolBarPlaceHolder::FindToolBarPlaceHolder(QWidget *owner, QWidget *parent)
    : QWidget(parent),
      owner(owner)
{
    setLayout(new QVBoxLayout);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    layout()->setContentsMargins(0, 0, 4, 0);
}

FindToolBarPlaceHolder::~FindToolBarPlaceHolder()
{
    PlaceHolderManager::instance()->removeHolder(this);
    if (findToolBar) {
        findToolBar->setVisible(false);
        findToolBar->setParent(nullptr);
    }
}

QWidget *FindToolBarPlaceHolder::getOwner() const
{
    return owner;
}

bool FindToolBarPlaceHolder::isUsedByWidget(QWidget *widget)
{
    QWidget *current = widget;
    while (current) {
        if (current == owner)
            return true;
        current = current->parentWidget();
    }
    return false;
}

void FindToolBarPlaceHolder::setWidget(FindToolBar *widget)
{
    if (findToolBar) {
        findToolBar->setVisible(false);
        findToolBar->setParent(nullptr);
    }

    findToolBar = widget;
    if (findToolBar)
        layout()->addWidget(findToolBar);
}

PlaceHolderManager *PlaceHolderManager::instance()
{
    static PlaceHolderManager ins;
    return &ins;
}

FindToolBarPlaceHolder *PlaceHolderManager::getCurrentHolder()
{
    return currentHolder;
}

void PlaceHolderManager::setCurrentHolder(FindToolBarPlaceHolder *holder)
{
    currentHolder = holder;
}

QWidget *PlaceHolderManager::createPlaceHolder(QWidget *owner, AbstractDocumentFind *docFind)
{
    auto holder = new FindToolBarPlaceHolder(owner, owner);
    holderInfo.insert(holder, docFind);

    return holder;
}

void PlaceHolderManager::removeHolder(FindToolBarPlaceHolder *holder)
{
    if (holder == currentHolder)
        currentHolder = nullptr;

    if (!holderInfo.contains(holder))
        return;

    holderInfo.remove(holder);
}

AbstractDocumentFind *PlaceHolderManager::findDocumentFind(FindToolBarPlaceHolder *holder) const
{
    if (!holderInfo.contains(holder))
        return nullptr;

    return holderInfo[holder];
}

const PlaceHolderManager::HolderInfo PlaceHolderManager::allHolderInfo()
{
    return holderInfo;
}
