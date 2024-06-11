// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemdelegate.h"
#include "notificationlistview.h"
#include "notificationitemwidget.h"
#include "notify/constants.h"

#include <DStyle>
#include <DFrame>

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QApplication>

DWIDGET_USE_NAMESPACE

ItemDelegate::ItemDelegate(NotificationListView *view, QObject *parent)
    : QStyledItemDelegate(parent),
      view(view)
{
}

QWidget *ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    EntityPtr notify = index.data().value<EntityPtr>();
    if (!notify)
        return nullptr;

    DFrame *frame = new DFrame(parent);
    frame->setBackgroundRole(DPalette::ItemBackground);
    frame->setFrameRounded(true);

    QHBoxLayout *layout = new QHBoxLayout(frame);
    layout->setContentsMargins(10, 10, 10, 10);

    NotificationItemWidget *item = new NotificationItemWidget(parent, notify);
    layout->addWidget(item);

    connect(item, &NotificationItemWidget::actionInvoked, view, std::bind(&NotificationListView::actionInvoked, view, notify, std::placeholders::_1));
    connect(item, &NotificationItemWidget::processed, view, &NotificationListView::processed);

    return frame;
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    EntityPtr notify = index.data().value<EntityPtr>();
    Q_ASSERT(notify);

    int height = 32;
    QFontMetrics fontMetrics(qApp->font());
    if (!notify->message().isEmpty()) {
        QRect boundingRect = fontMetrics.boundingRect(0, 0, 300, 0, Qt::TextWordWrap, notify->message());
        height += boundingRect.height() + 20;
    }

    if (!notify->name().isEmpty()) {
        QRect boundingRect = fontMetrics.boundingRect(0, 0, 300, 0, Qt::TextWordWrap, notify->name());
        height += boundingRect.height() + 10;
    }

    if (!notify->actions().isEmpty()) {
        height += 34;
    }

    return { view->width(), height };
}

void ItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    QRect rect = option.rect;
    QSize size = sizeHint(option, index);
    editor->setGeometry(rect.x(), rect.y(), size.width(), size.height() - ItemSpacing);
}
