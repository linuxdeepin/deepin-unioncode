// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notificationitemwidget.h"
#include "actionbutton.h"

#include <DIconButton>
#include <DLabel>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class NotificationItemWidgetPrivate
{
public:
    explicit NotificationItemWidgetPrivate(NotificationItemWidget *qq);

    void initUI();
    void initConnection();

    QWidget *createCenterWidget();
    QWidget *createBottomWidget();

    void updateContent();

public:
    NotificationItemWidget *q;

    EntityPtr entity;
    QTimer *timer { nullptr };
    DLabel *msgLabel { nullptr };
    DLabel *fromLabel { nullptr };
    DIconButton *closeBtn { nullptr };
    DIconButton *expandFoldBtn { nullptr };
    DIconButton *iconBtn { nullptr };
    ActionButton *actButton { nullptr };
};

NotificationItemWidgetPrivate::NotificationItemWidgetPrivate(NotificationItemWidget *qq)
    : q(qq)
{
}

void NotificationItemWidgetPrivate::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);

    mainLayout->addWidget(createCenterWidget());
    mainLayout->addWidget(createBottomWidget());
}

void NotificationItemWidgetPrivate::initConnection()
{
    q->connect(actButton, &ActionButton::buttonClicked, q,
               [this](const QString &actId) {
                   Q_EMIT q->actionInvoked(actId);
                   Q_EMIT q->processed(entity);
               });
    q->connect(closeBtn, &DIconButton::clicked, q,
               [this]() {
                   Q_EMIT q->dismissed();
                   Q_EMIT q->processed(entity);
               });
}

QWidget *NotificationItemWidgetPrivate::createCenterWidget()
{
    QWidget *widget = new QWidget(q);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    iconBtn = new DIconButton(q);
    iconBtn->setIconSize({ 20, 20 });
    iconBtn->setFlat(true);
    iconBtn->setEnabledCircle(true);

    msgLabel = new DLabel(q);
    msgLabel->setWordWrap(true);

    fromLabel = new DLabel(q);
    fromLabel->setForegroundRole(DPalette::PlaceholderText);

    closeBtn = new DIconButton(q);
    closeBtn->setFlat(true);
    closeBtn->setEnabledCircle(true);
    closeBtn->setIconSize({ 32, 32 });
    closeBtn->setIcon(DStyle::SP_CloseButton);

    QVBoxLayout *msgLayout = new QVBoxLayout;
    msgLayout->setContentsMargins(0, 0, 0, 0);
    msgLayout->setSpacing(10);
    msgLayout->addWidget(msgLabel);
    msgLayout->addWidget(fromLabel);

    layout->addWidget(iconBtn, 0, Qt::AlignLeft | Qt::AlignTop);
    layout->addLayout(msgLayout, 1);
    layout->addWidget(closeBtn, 0, Qt::AlignRight | Qt::AlignTop);

    return widget;
}

QWidget *NotificationItemWidgetPrivate::createBottomWidget()
{
    QWidget *widget = new QWidget(q);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    actButton = new ActionButton(q);

    layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
    layout->addWidget(actButton);

    return widget;
}

void NotificationItemWidgetPrivate::updateContent()
{
    msgLabel->setText(entity->message());

    QIcon icon;
    switch (entity->type()) {
    case NotificationEntity::Information:
        icon = QIcon::fromTheme("notification_info");
        break;
    case NotificationEntity::Warning:
        icon = QIcon::fromTheme("notification_warning");
        break;
    case NotificationEntity::Error:
        icon = QIcon::fromTheme("notification_error");
        break;
    }
    iconBtn->setIcon(icon);

    if (entity->actions().isEmpty())
        actButton->setVisible(false);
    else
        actButton->addButtons(entity->actions());

    if (entity->name().isEmpty()) {
        fromLabel->setVisible(false);
    } else {
        QString format(NotificationItemWidget::tr("Source: %1"));
        fromLabel->setText(format.arg(entity->name()));
    }
}

NotificationItemWidget::NotificationItemWidget(QWidget *parent, EntityPtr entity)
    : QWidget(parent),
      d(new NotificationItemWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();

    setEntity(entity);
}

NotificationItemWidget::~NotificationItemWidget()
{
    delete d;
}

EntityPtr NotificationItemWidget::entity() const
{
    return d->entity;
}

void NotificationItemWidget::setEntity(EntityPtr entity)
{
    if (!entity)
        return;

    d->entity = entity;
    d->updateContent();
}
