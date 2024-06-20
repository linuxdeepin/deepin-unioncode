// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
