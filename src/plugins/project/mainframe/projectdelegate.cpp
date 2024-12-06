// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectdelegate.h"
#include "services/project/projectservice.h"

#include <DGuiApplicationHelper>
#include <DSpinner>

#include <QStyledItemDelegate>
#include <QAbstractItemView>
#include <QPainter>

DGUI_USE_NAMESPACE

class ProjectDelegatePrivate
{
    friend class ProjectDelegate;
    QModelIndex index;
    Dtk::Widget::DSpinner *spinner { nullptr };
};

ProjectDelegate::ProjectDelegate(QAbstractItemView *parent)
    : BaseItemDelegate(parent), d(new ProjectDelegatePrivate)
{
    d->spinner = new Dtk::Widget::DSpinner(parent);
    d->spinner->setFixedSize(16, 16);
    d->spinner->hide();
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
        d->spinner->move(option.rect.right() - 20, option.rect.top() + 4);

        if (index.data(ProjectItemRole::ParsingStateRole).value<ParsingState>() == ParsingState::Wait && !d->spinner->isVisible()) {
            d->spinner->show();
            d->spinner->start();
        } else if (index.data(ProjectItemRole::ParsingStateRole).value<ParsingState>() == ParsingState::Done) {
            d->spinner->hide();
            d->spinner->stop();
        }
    }

    BaseItemDelegate::paint(painter, iOption, index);
}

QSize ProjectDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto size = BaseItemDelegate::sizeHint(option, index);
    size.setHeight(24);
    return size;
}

void ProjectDelegate::hideSpinner()
{
    d->spinner->hide();
    d->spinner->stop();
}
