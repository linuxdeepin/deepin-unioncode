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
#ifndef BREAKPOINTITEM_H
#define BREAKPOINTITEM_H

#include "breakpoint.h"

#include <QTreeView>
#include <QObject>

enum StackColumns {
    kIndexColumn,
    kFunctionNameColumn,
    kFileNameColumn,
    kLineNumberColumn,
    kAddressColumn,
    kStackColumnCount
};

class BreakpointItem : public QObject
{
    Q_OBJECT
public:
    explicit BreakpointItem(const Internal::Breakpoint &_bp);
    explicit BreakpointItem(const BreakpointItem &_bp);
    BreakpointItem();
    ~BreakpointItem();

    QVariant data(int row, int column, int role) const;
    QString markerFileName() const;
    int markerLineNumber() const;

    int modelId() const;
    QString displayName() const;
    QString toolTip() const;

    int lineNumber() const;
    bool isEnabled() const;

    void setEnabled(bool on);

    const Internal::Breakpoint &breakpoint() const {return bp;}

    BreakpointItem &operator=(const BreakpointItem &item)
    {
        bp = item.breakpoint();
        return *this;
    }

    bool operator==(const BreakpointItem &item)
    {
        return bp == item.breakpoint();
    }

private:
    Internal::Breakpoint bp;
};

using BreakpointItems = QVector<BreakpointItem>;
#endif // BREAKPOINTITEM_H
