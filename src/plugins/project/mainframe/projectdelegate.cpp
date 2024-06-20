// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectdelegate.h"

#include <DGuiApplicationHelper>
#include <QStyledItemDelegate>
#include <QAbstractItemView>

#include <QPainter>

DGUI_USE_NAMESPACE

class ProjectDelegatePrivate
{
    friend class ProjectDelegate;
    QModelIndex index;
};

ProjectDelegate::ProjectDelegate(QAbstractItemView *parent)
    : BaseItemDelegate(parent), d(new ProjectDelegatePrivate)
{
}

ProjectDelegate::~ProjectDelegate()
{
    if (d)
        delete d;
}

void ProjectDelegate::setActiveProject(const QModelIndex &root)
{
    d->index = root;
}

const QModelIndex &ProjectDelegate::getActiveProject() const
{
    return d->index;
}

void ProjectDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    QStyleOptionViewItem iOption = option;
    if (d->index.isValid() && d->index == index) {
        iOption.font.setBold(true);
    }

    BaseItemDelegate::paint(painter, iOption, index);
}

QSize ProjectDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    return { option.rect.width(), 24 };
}
