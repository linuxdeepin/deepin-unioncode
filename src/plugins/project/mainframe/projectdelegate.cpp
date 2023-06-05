/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#include "projectdelegate.h"

#include <QPainter>

class ProjectDelegatePrivate
{
    friend class ProjectDelegate;
    QModelIndex index;
};

ProjectDelegate::ProjectDelegate(QObject *parent)
  : QStyledItemDelegate (parent)
  , d (new ProjectDelegatePrivate)
{

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
    QStyledItemDelegate::paint(painter, iOption, index);
}
