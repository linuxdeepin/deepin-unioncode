// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notificationmanager.h"
#include "constants.h"
#include "gui/bubble.h"
#include "gui/notificationcenterwidget.h"
#include "uicontroller/controller.h"

#include "common/util/eventdefinitions.h"

#include <QStatusBar>
#include <QApplication>
#include <QWindow>
#include <QPointer>
#include <QPropertyAnimation>
#include <QVBoxLayout>

class NotificationManagerPrivate
{
public:
    explicit NotificationManagerPrivate(NotificationManager *qq);

    void pushBubble(EntityPtr notify);
    Bubble *createBubble(EntityPtr notify);
    void addContent(QWidget *widget);

public:
    NotificationManager *q;

    NotificationCenterWidget *ncWidget { nullptr };
    QWidget *notificationContent { nullptr };
    MainWindow *mainWindow { nullptr };
    int replaceCount { 0 };
    QList<EntityPtr> allEntities;
    QList<QPointer<Bubble>> displayedBubbleList;
};

NotificationManagerPrivate::NotificationManagerPrivate(NotificationManager *qq)
    : q(qq)
{
}

void NotificationManagerPrivate::pushBubble(EntityPtr notify)
{
    if (!notify)
        return;

    if (!mainWindow)
        mainWindow = Controller::instance()->mainWindow();

    Bubble *bubble = createBubble(notify);
    if (!bubble)
        return;

    addContent(bubble);
}

Bubble *NotificationManagerPrivate::createBubble(EntityPtr notify)
{
    allEntities.push_front(notify);
    if (displayedBubbleList.size() >= BubbleEntities) {
        Q_EMIT q->updated();
        return nullptr;
    }

    Bubble *bubble = new Bubble(mainWindow, notify);
    q->connect(bubble, &Bubble::expired, q, &NotificationManager::bubbleExpired);
    q->connect(bubble, &Bubble::dismissed, q, &NotificationManager::bubbleDismissed);
    q->connect(bubble, &Bubble::actionInvoked, q, &NotificationManager::bubbleActionInvoked);
    q->connect(bubble, &Bubble::processed, q, [this](EntityPtr ptr) {
        allEntities.removeOne(ptr);
    });

    displayedBubbleList << bubble;
    return bubble;
}

void NotificationManagerPrivate::addContent(QWidget *widget)
{
    if (!notificationContent) {
        notificationContent = new QWidget(mainWindow);
        notificationContent->setFixedWidth(420);
        notificationContent->setObjectName(NotificationContentName);
        notificationContent->setWindowFlag(Qt::WindowStaysOnTopHint, true);
        notificationContent->setAttribute(Qt::WA_AlwaysStackOnTop);
        notificationContent->installEventFilter(q);
        mainWindow->installEventFilter(q);
        QVBoxLayout *layout = new QVBoxLayout(notificationContent);
        layout->setSpacing(0);
        layout->setMargin(0);
        layout->setDirection(QBoxLayout::BottomToTop);
        notificationContent->show();
    }

    static_cast<QBoxLayout *>(notificationContent->layout())->addWidget(widget, 0, Qt::AlignHCenter);
}

NotificationManager::NotificationManager(QObject *parent)
    : QObject(parent),
      d(new NotificationManagerPrivate(this))
{
}

NotificationManager::~NotificationManager()
{
    delete d;
}

NotificationManager *NotificationManager::instance()
{
    static NotificationManager ins;
    return &ins;
}

void NotificationManager::notify(uint type, const QString &name, const QString &msg, const QStringList &actions)
{
    EntityPtr notification = std::make_shared<NotificationEntity>(static_cast<NotificationEntity::NotificationType>(type),
                                                                  name, msg, actions);

    if (d->ncWidget && d->ncWidget->isVisible()) {
        d->allEntities.push_front(notification);
        d->ncWidget->addNotification(notification);
        return;
    }

    d->pushBubble(notification);
}

void NotificationManager::show()
{
    if (d->ncWidget && d->ncWidget->isVisible())
        return;

    if (!d->mainWindow)
        d->mainWindow = Controller::instance()->mainWindow();

    if (!d->ncWidget)
        d->ncWidget = new NotificationCenterWidget(d->mainWindow);

    for (auto bubble : d->displayedBubbleList)
        bubble->close();

    d->displayedBubbleList.clear();
    d->ncWidget->setNotifications(d->allEntities);
    d->addContent(d->ncWidget);
}

void NotificationManager::hide()
{
    if (!d->ncWidget || !d->ncWidget->isVisible())
        return;

    d->ncWidget->close();
    d->ncWidget->deleteLater();
    d->ncWidget = nullptr;
}

void NotificationManager::toggle()
{
    if (d->ncWidget && d->ncWidget->isVisible())
        hide();
    else
        show();
}

void NotificationManager::clear()
{
    d->allEntities.clear();
}

void NotificationManager::remove(EntityPtr entity)
{
    if (!d->allEntities.contains(entity))
        return;

    d->allEntities.removeOne(entity);
}

void NotificationManager::bubbleExpired(Bubble *bubble)
{
    if (d->displayedBubbleList.contains(bubble)) {
        d->displayedBubbleList.removeOne(bubble);
        bubble->close();
    }

    Q_EMIT updated();
}

void NotificationManager::bubbleDismissed(Bubble *bubble)
{
    if (d->displayedBubbleList.contains(bubble)) {
        d->displayedBubbleList.removeOne(bubble);
        bubble->close();
    }
}

void NotificationManager::bubbleActionInvoked(Bubble *bubble, const QString &actId)
{
    if (d->displayedBubbleList.contains(bubble)) {
        d->displayedBubbleList.removeOne(bubble);
        bubble->close();
    }

    notifyManager.actionInvoked(actId);
}

bool NotificationManager::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::LayoutRequest || event->type() == QEvent::Resize) {
        if (QWidget *widget = qobject_cast<QWidget *>(watched)) {
            QWidget *content { nullptr };
            if (widget->objectName() == NotificationContentName) {
                content = widget;
            } else {
                content = widget->findChild<QWidget *>(NotificationContentName, Qt::FindDirectChildrenOnly);
            }

            QWidget *par = content->parentWidget();
            QRect geometry(QPoint(0, 0), content->sizeHint());
            geometry.moveRight(par->rect().right());
            geometry.moveBottom(par->rect().bottom() - d->mainWindow->statusBar()->height() - StatusBarPadding);
            content->setGeometry(geometry);
            content->raise();
        }
    } else if (event->type() == QEvent::ChildRemoved) {
        // 如果是通知消息被删除的事件
        if (QWidget *widget = qobject_cast<QWidget *>(watched)) {
            if (widget == d->notificationContent
                && widget->layout() && widget->layout()->count() == 0) {
                widget->parent()->removeEventFilter(this);
                d->notificationContent->deleteLater();
                d->notificationContent = nullptr;
            }
        }
    }

    return QObject::eventFilter(watched, event);
}
