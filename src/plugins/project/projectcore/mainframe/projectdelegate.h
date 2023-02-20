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
#ifndef PROJECTDELEGATE_H
#define PROJECTDELEGATE_H

#include <QStyledItemDelegate>

class ProjectDelegatePrivate;
class ProjectDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    ProjectDelegatePrivate *const d;
public:
    explicit ProjectDelegate(QObject *parent = nullptr);
    void setActiveProject(const QModelIndex &root);
    const QModelIndex &getActiveProject() const;

protected:
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

#endif // PROJECTDELEGATE_H
