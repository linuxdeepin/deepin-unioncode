// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionbutton.h"
#include "notify/constants.h"

#include <DSuggestButton>

#include <QHBoxLayout>
#include <QPushButton>

#include <functional>

DWIDGET_USE_NAMESPACE

class ActionButtonPrivate
{
public:
    explicit ActionButtonPrivate(ActionButton *qq);

    void initUI();

public:
    ActionButton *q;

    QList<QAbstractButton *> buttons;
};

ActionButtonPrivate::ActionButtonPrivate(ActionButton *qq)
    : q(qq)
{
}

void ActionButtonPrivate::initUI()
{
    QHBoxLayout *layout = new QHBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    layout->setAlignment(Qt::AlignRight);
    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

ActionButton::ActionButton(QWidget *parent)
    : QWidget(parent),
      d(new ActionButtonPrivate(this))
{
    d->initUI();
}

ActionButton::~ActionButton()
{
    delete d;
}

bool ActionButton::addButtons(const QStringList &list)
{
    if (list.isEmpty())
        return false;

    QString id;
    for (int i = 0; i != list.size(); ++i) {
        if (i % 2 == 0) {
            id = list[i];
        } else {
            QAbstractButton *button { nullptr };
            if (id.endsWith(DefaultButtonField))
                button = new DSuggestButton(this);
            else
                button = new QPushButton(this);

            button->setText(list[i]);
            layout()->addWidget(button);

            connect(button, &QPushButton::clicked, this, std::bind(&ActionButton::buttonClicked, this, id));
            d->buttons << button;
        }
    }

    return true;
}

QList<QAbstractButton *> ActionButton::buttonList()
{
    return d->buttons;
}

bool ActionButton::isEmpty()
{
    return d->buttons.isEmpty();
}

void ActionButton::clear()
{
    for (auto btn : d->buttons) {
        btn->hide();
        layout()->removeWidget(btn);
        btn->deleteLater();
    }

    d->buttons.clear();
}
