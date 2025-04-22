// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUBBLE_H
#define BUBBLE_H

#include "notify/constants.h"

#include <DFloatingWidget>

class BubblePrivate;
class Bubble : public DTK_WIDGET_NAMESPACE::DFloatingWidget
{
    Q_OBJECT
public:
    explicit Bubble(QWidget *parent = nullptr, EntityPtr entity = nullptr);
    ~Bubble();

    EntityPtr entity() const;
    void setEntity(EntityPtr entity);

Q_SIGNALS:
    void expired(Bubble *);
    void dismissed(Bubble *);
    void processed(EntityPtr ptr);
    void actionInvoked(Bubble *, QString);

protected:
    virtual void showEvent(QShowEvent *event) override;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    virtual void enterEvent(QEvent *event) override;
#else
    virtual void enterEvent(QEnterEvent *event) override;
#endif
    virtual void leaveEvent(QEvent *event) override;

private:
    BubblePrivate *const d;
};

#endif   // BUBBLE_H
