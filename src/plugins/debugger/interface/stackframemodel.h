// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
