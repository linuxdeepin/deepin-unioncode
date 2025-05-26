// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bubble.h"
#include "notificationitemwidget.h"

#include <QTimer>
#include <QPointer>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class BubblePrivate
{
public:
    explicit BubblePrivate(Bubble *qq);

    void initUI();
    void initConnection();
    void initTimers();

public:
    Bubble *q;

    QTimer *timer { nullptr };
    NotificationItemWidget *itemWidget { nullptr };
};

BubblePrivate::BubblePrivate(Bubble *qq)
    : q(qq)
{
}

void BubblePrivate::initUI()
{
    q->setFixedWidth(420);
    q->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    itemWidget = new NotificationItemWidget(q);
    mainLayout->addWidget(itemWidget);
}

void BubblePrivate::initConnection()
{
    q->connect(itemWidget, &NotificationItemWidget::dismissed, q, std::bind(&Bubble::dismissed, q, q));
    q->connect(itemWidget, &NotificationItemWidget::processed, q, &Bubble::processed);
    q->connect(itemWidget, &NotificationItemWidget::actionInvoked, q, std::bind(&Bubble::actionInvoked, q, q, std::placeholders::_1));
    q->connect(timer, &QTimer::timeout, q, std::bind(&Bubble::expired, q, q));
}

void BubblePrivate::initTimers()
{
    timer = new QTimer(q);
    timer->setInterval(BubbleTimeout);
    timer->setSingleShot(true);
}

Bubble::Bubble(QWidget *parent, EntityPtr entity)
    : DFloatingWidget(parent),
      d(new BubblePrivate(this))
{
    d->initUI();
    d->initTimers();
    d->initConnection();

    setEntity(entity);
}

Bubble::~Bubble()
{
    delete d;
}

EntityPtr Bubble::entity() const
{
    return d->itemWidget->entity();
}

void Bubble::setEntity(EntityPtr entity)
{
    if (!entity)
        return;

    d->itemWidget->setEntity(entity);
}

void Bubble::showEvent(QShowEvent *event)
{
    if (d->timer)
        d->timer->start();

    DFloatingWidget::showEvent(event);
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
void Bubble::enterEvent(QEvent *event)
#else
void Bubble::enterEvent(QEnterEvent *event)
#endif
{
    if (d->timer && d->timer->isActive())
        d->timer->stop();

    DFloatingWidget::enterEvent(event);
}

void Bubble::leaveEvent(QEvent *event)
{
    if (d->timer && !d->timer->isActive())
        d->timer->start();

    DFloatingWidget::leaveEvent(event);
}
