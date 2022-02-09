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
#ifndef BREAKPOINTMODEL_H
#define BREAKPOINTMODEL_H

#include "breakpoint.h"
#include "breakpointitem.h"
#include "stackframemodel.h"

#include <QAbstractItemModel>

class BreakpointModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit BreakpointModel(QObject *parent = nullptr);
    ~BreakpointModel() override;

    void setBreakpoints(const Internal::Breakpoints &breakpoints, bool canExpand = false);
    void insertBreakpoint(const Internal::Breakpoint &breakpoint);
    void removeBreakpoint(const Internal::Breakpoint &breakpoint);

    void setCurrentIndex(int index);
    const Internal::Breakpoint &BreakpointAt(int index) const{ return bps.at(index).breakpoint(); }
    int breakpointSize() const { return bps.size(); }
    void removeAll();
    QAbstractItemModel *model() { return this; }
    bool isContentsValid() const { return contentsValid; }
    Internal::Breakpoint currentBreakpoint() const;
signals:
    void breakpointChanged();
    void currentIndexChanged();

private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &idx, const QVariant &data, int role) override;

//    Internal::Breakpoints bps;
    BreakpointItems bps;
    int currentIndex = -1;
    bool canExpand = false;
    bool contentsValid = false;
};

#endif // BREAKPOINTMODEL_H
