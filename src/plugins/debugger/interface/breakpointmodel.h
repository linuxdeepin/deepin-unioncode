// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    void switchBreakpointStatus(const Internal::Breakpoint &breakpoint);
    void setBreakpointCondition(const Internal::Breakpoint &breakpoint);

    void setCurrentIndex(int index);
    const Internal::Breakpoint &BreakpointAt(int index) const{ return bps.at(index).breakpoint(); }
    int breakpointSize() const { return bps.size(); }
    void removeAll();
    QAbstractItemModel *model() { return this; }
    bool isContentsValid() const { return contentsValid; }
    Internal::Breakpoint currentBreakpoint() const;
    const BreakpointItems &breakpointList() const;
signals:
    void breakpointChanged();
    void currentIndexChanged();

private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

//    Internal::Breakpoints bps;
    BreakpointItems bps;
    int currentIndex = -1;
    bool canExpand = false;
    bool contentsValid = false;
};

#endif // BREAKPOINTMODEL_H
