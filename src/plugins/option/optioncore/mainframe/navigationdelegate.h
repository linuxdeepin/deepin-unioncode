// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QScopedPointer>

#include <DStyle>
#include <DStyledItemDelegate>

DWIDGET_BEGIN_NAMESPACE

class NavigationDelegatePrivate;
class NavigationDelegate: public DStyledItemDelegate
{
    Q_OBJECT
public:
    NavigationDelegate(QAbstractItemView *parent);
    ~NavigationDelegate();


    enum DataRole {
        NavLevelRole = Dtk::UserRole + 100,
        NavKeyRole,
    };

    enum NavLevel {
        Split = 1001,
        Level1,
        Level2,
        Level3
    };

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};

DWIDGET_END_NAMESPACE
