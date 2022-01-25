/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#ifndef STACKHANDLER_H
#define STACKHANDLER_H

#include "stackframe.h"

#include <QAbstractItemModel>

enum {
    ItemActivatedRole = Qt::UserRole,
    ItemClickedRole
};

class StackFrameModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit StackFrameModel();
    ~StackFrameModel() override;

    void setFrames(const StackFrames &frames, bool canExpand = false);
    void setCurrentIndex(int index);
    const StackFrameData &frameAt(int index) const { return stackFrames.at(index); }
    int stackSize() const { return stackFrames.size(); }
    void removeAll();
    QAbstractItemModel *model() { return this; }
    bool isContentsValid() const { return contentsValid; }
    StackFrameData currentFrame() const;
signals:
    void stackChanged();
    void currentIndexChanged();

private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &idx, const QVariant &data, int role) override;

    StackFrames stackFrames;
    int currentIndex = -1;
    bool canExpand = false;
    bool contentsValid = false;
};

#endif   // STACKHANDLER_H
