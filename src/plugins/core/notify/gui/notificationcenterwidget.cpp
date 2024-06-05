// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notificationcenterwidget.h"
#include "itemdelegate.h"
#include "notificationlistview.h"
#include "notificationmodel.h"
#include "notify/notificationmanager.h"

#include "common/util/eventdefinitions.h"

#include <DLabel>
#include <DToolButton>
#include <DIconTheme>

#include <QStackedLayout>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class NotificationCenterWidgetPrivate
{
public:
    explicit NotificationCenterWidgetPrivate(NotificationCenterWidget *qq);

    void initUI();
    void initConntction();
    void updateView();

public:
    NotificationCenterWidget *q;

    DToolButton *hideBtn { nullptr };
    DToolButton *clearBtn { nullptr };
    QStackedLayout *contentLayout { nullptr };

    NotificationListView *notifyView { nullptr };
    NotificationModel *viewModel { nullptr };
};

NotificationCenterWidgetPrivate::NotificationCenterWidgetPrivate(NotificationCenterWidget *qq)
    : q(qq)
{
}

void NotificationCenterWidgetPrivate::initUI()
{
    q->setFixedSize(420, 368);
    q->setBackgroundRole(QPalette::Base);

    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    QLabel *titleLabel = new QLabel(NotificationCenterWidget::tr("Notification"), q);
    hideBtn = new DToolButton(q);
    hideBtn->setIconSize({ 16, 16 });
    hideBtn->setIcon(DIconTheme::findQIcon("hide"));

    clearBtn = new DToolButton(q);
    clearBtn->setIconSize({ 16, 16 });
    clearBtn->setIcon(DIconTheme::findQIcon("clear_history"));

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setContentsMargins(10, 0, 10, 0);
    titleLayout->addWidget(titleLabel);
    titleLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
    titleLayout->addWidget(hideBtn);
    titleLayout->addWidget(clearBtn);

    notifyView = new NotificationListView(q);
    viewModel = new NotificationModel(q, notifyView);
    notifyView->setModel(viewModel);
    notifyView->setItemDelegate(new ItemDelegate(notifyView, q));
    notifyView->setAutoFillBackground(false);
    notifyView->viewport()->setAutoFillBackground(false);
    notifyView->setFrameStyle(QFrame::NoFrame);
    notifyView->setMouseTracking(true);
    notifyView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    notifyView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    notifyView->setVerticalScrollMode(QListView::ScrollPerPixel);
    notifyView->setContentsMargins(0, 0, 0, 0);
    notifyView->setUpdatesEnabled(true);
    notifyView->setSelectionMode(QListView::NoSelection);
    notifyView->setFocusPolicy(Qt::NoFocus);

    DLabel *noNotificationLabel = new DLabel(NotificationCenterWidget::tr("No new notifications"), q);
    noNotificationLabel->setForegroundRole(DPalette::PlaceholderText);
    noNotificationLabel->setAlignment(Qt::AlignCenter);

    contentLayout = new QStackedLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->addWidget(noNotificationLabel);
    contentLayout->addWidget(notifyView);

    mainLayout->addLayout(titleLayout);
    mainLayout->addLayout(contentLayout, 1);
}

void NotificationCenterWidgetPrivate::initConntction()
{
    q->connect(hideBtn, &DToolButton::clicked, NotificationManager::instance(), &NotificationManager::hide);
    q->connect(clearBtn, &DToolButton::clicked, q, &NotificationCenterWidget::handleClear);
    q->connect(notifyView, &NotificationListView::actionInvoked, q, &NotificationCenterWidget::handleActionInvoked);
    q->connect(notifyView, &NotificationListView::processed, q, &NotificationCenterWidget::handleProcessed);
}

void NotificationCenterWidgetPrivate::updateView()
{
    contentLayout->setCurrentIndex(viewModel->rowCount() == 0 ? 0 : 1);
}

NotificationCenterWidget::NotificationCenterWidget(QWidget *parent)
    : DFloatingWidget(parent),
      d(new NotificationCenterWidgetPrivate(this))
{
    d->initUI();
    d->initConntction();
}

NotificationCenterWidget::~NotificationCenterWidget()
{
    delete d;
}

void NotificationCenterWidget::setNotifications(const QList<EntityPtr> &datas)
{
    d->viewModel->setNotifications(datas);
    d->updateView();
}

void NotificationCenterWidget::addNotification(EntityPtr data)
{
    d->viewModel->addNotification(data);
    d->updateView();
}

void NotificationCenterWidget::handleClear()
{
    NotificationManager::instance()->clear();
    NotificationManager::instance()->hide();
    d->updateView();
}

void NotificationCenterWidget::handleActionInvoked(const QString &actId)
{
    notifyManager.actionInvoked(actId);
}

void NotificationCenterWidget::handleProcessed(EntityPtr ptr)
{
    d->viewModel->removeNotification(ptr);
    NotificationManager::instance()->remove(ptr);
    d->updateView();
}
