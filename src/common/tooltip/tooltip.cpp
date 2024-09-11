// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tooltip.h"
#include "tips.h"

#include <QTimer>
#include <QScreen>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QApplication>

class ToolTipPrivate : public QObject
{
public:
    explicit ToolTipPrivate(ToolTip *qq);
    ~ToolTipPrivate();

    void showTip(const QPoint &pos, const QVariant &content, TipLabel::ContentType type, QWidget *w, const QRect &rect);
    void hideTip();
    void delayHideTip();
    bool acceptShow(const QVariant &content, TipLabel::ContentType type, const QPoint &pos, QWidget *w, const QRect &rect);
    void setUp(const QPoint &pos, QWidget *w, const QRect &rect);
    bool tipChanged(const QPoint &pos, TipLabel::ContentType type, const QVariant &content, QWidget *w) const;
    void setTipRect(QWidget *w, const QRect &rect);
    void placeTip(const QPoint &pos);

public:
    ToolTip *q;

    TipLabel *tipLabel { nullptr };
    QWidget *widget { nullptr };
    QRect rect;
    QTimer showTimer;
    QTimer hideDelayTimer;
};

ToolTipPrivate::ToolTipPrivate(ToolTip *qq)
    : q(qq)
{
    connect(&showTimer, &QTimer::timeout, this, &ToolTipPrivate::hideTip);
    connect(&hideDelayTimer, &QTimer::timeout, this, &ToolTipPrivate::hideTip);
}

ToolTipPrivate::~ToolTipPrivate()
{
    tipLabel = nullptr;
}

void ToolTipPrivate::showTip(const QPoint &pos, const QVariant &content, TipLabel::ContentType type, QWidget *w, const QRect &rect)
{
    if (acceptShow(content, type, pos, w, rect)) {
        switch (type) {
        case TipLabel::TextContent:
            tipLabel = new TextTip(w);
            break;
        case TipLabel::WidgetContent:
            tipLabel = new WidgetTip(w);
            break;
        }

        tipLabel->setObjectName("ToolTip");
        tipLabel->setContent(content);
        setUp(pos, w, rect);
        qApp->installEventFilter(q);
        tipLabel->show();
    }
}

void ToolTipPrivate::hideTip()
{
    if (tipLabel) {
        tipLabel->close();
        tipLabel->deleteLater();
        tipLabel = nullptr;
    }

    showTimer.stop();
    hideDelayTimer.stop();
    qApp->removeEventFilter(q);
    Q_EMIT q->hidden();
}

void ToolTipPrivate::delayHideTip()
{
    if (!hideDelayTimer.isActive())
        hideDelayTimer.start(300);
}

bool ToolTipPrivate::acceptShow(const QVariant &content, TipLabel::ContentType type, const QPoint &pos, QWidget *w, const QRect &rect)
{
    if (q->isVisible()) {
        if (tipLabel->canHandleContentReplacement(type)) {
            QPoint localPos = pos;
            if (w)
                localPos = w->mapFromGlobal(pos);
            if (tipChanged(localPos, type, content, w)) {
                tipLabel->setContent(content);
                setUp(pos, w, rect);
            }
            return false;
        }
        hideTip();
    }

    return true;
}

void ToolTipPrivate::setUp(const QPoint &pos, QWidget *w, const QRect &rect)
{
    tipLabel->configure(pos);

    placeTip(pos);
    setTipRect(w, rect);

    if (hideDelayTimer.isActive())
        hideDelayTimer.stop();
    showTimer.start(tipLabel->showTime());
}

bool ToolTipPrivate::tipChanged(const QPoint &pos, TipLabel::ContentType type, const QVariant &content, QWidget *w) const
{
    if (!tipLabel->equals(type, content) || widget != w)
        return true;
    if (!rect.isNull())
        return !rect.contains(pos);
    return false;
}

void ToolTipPrivate::setTipRect(QWidget *w, const QRect &rect)
{
    if (!this->rect.isNull() && !w) {
        qWarning("ToolTip::show: Cannot pass null widget if rect is set");
    } else {
        widget = w;
        this->rect = rect;
    }
}

void ToolTipPrivate::placeTip(const QPoint &pos)
{
    QScreen *qscreen = QGuiApplication::screenAt(pos);
    if (!qscreen)
        qscreen = QGuiApplication::primaryScreen();

    const QRect screen = qscreen->availableGeometry();
    QPoint p = pos;
    p += { 2, 16 };
    if (p.x() + tipLabel->width() > screen.x() + screen.width())
        p.rx() -= 4 + tipLabel->width();
    if (p.y() + tipLabel->height() > screen.y() + screen.height())
        p.ry() -= 24 + tipLabel->height();
    if (p.y() < screen.y())
        p.setY(screen.y());
    if (p.x() + tipLabel->width() > screen.x() + screen.width())
        p.setX(screen.x() + screen.width() - tipLabel->width());
    if (p.x() < screen.x())
        p.setX(screen.x());
    if (p.y() + tipLabel->height() > screen.y() + screen.height())
        p.setY(screen.y() + screen.height() - tipLabel->height());

    tipLabel->move(p);
}

ToolTip::ToolTip()
    : d(new ToolTipPrivate(this))
{
}

ToolTip::~ToolTip()
{
    delete d;
}

ToolTip *ToolTip::instance()
{
    static ToolTip ins;
    return &ins;
}

void ToolTip::show(const QPoint &pos, const QString &content, QWidget *w, const QRect &rect)
{
    if (content.isEmpty())
        hide();
    else
        instance()->d->showTip(pos, content, TipLabel::TextContent, w, rect);
}

void ToolTip::show(const QPoint &pos, QWidget *content, QWidget *w, const QRect &rect)
{
    if (!content)
        hide();
    else
        instance()->d->showTip(pos, QVariant::fromValue(content), TipLabel::WidgetContent, w, rect);
}

void ToolTip::show(const QPoint &pos, QLayout *content, QWidget *w, const QRect &rect)
{
    if (content && content->count()) {
        auto tooltipWidget = new QWidget;
        QScreen *qscreen = QGuiApplication::screenAt(pos);
        if (!qscreen)
            qscreen = QGuiApplication::primaryScreen();
        tooltipWidget->setMaximumSize(qscreen->availableSize() * 0.9);
        tooltipWidget->setLayout(content);
        instance()->d->showTip(pos, QVariant::fromValue(tooltipWidget), TipLabel::WidgetContent, w, rect);
    } else {
        hide();
    }
}

void ToolTip::hide()
{
    instance()->d->delayHideTip();
}

void ToolTip::hideImmediately()
{
    instance()->d->hideTip();
}

bool ToolTip::isVisible()
{
    ToolTipPrivate *p = instance()->d;
    return p->tipLabel && p->tipLabel->isVisible();
}

bool ToolTip::eventFilter(QObject *obj, QEvent *event)
{
    if (d->tipLabel && event->type() == QEvent::ApplicationStateChange
        && QGuiApplication::applicationState() != Qt::ApplicationActive) {
        d->hideTip();
    }

    if (!obj->isWidgetType())
        return false;

    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        int key = static_cast<QKeyEvent *>(event)->key();
        if (key == Qt::Key_Escape)
            d->hideTip();
        break;
    }
    case QEvent::Leave:
        if (obj == d->tipLabel && !d->tipLabel->isAncestorOf(QApplication::focusWidget()))
            d->delayHideTip();
        break;
    case QEvent::Enter:
        if (d->tipLabel && d->tipLabel->isInteractive() && obj == d->tipLabel) {
            if (d->hideDelayTimer.isActive())
                d->hideDelayTimer.stop();
        }
        break;
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
    case QEvent::FocusOut:
    case QEvent::FocusIn:
        if (d->tipLabel && !d->tipLabel->isInteractive())
            d->hideTip();
        break;
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::Wheel:
        if (d->tipLabel) {
            if (d->tipLabel->isInteractive()) {
                if (obj != d->tipLabel && !d->tipLabel->isAncestorOf(static_cast<QWidget *>(obj)))
                    d->hideTip();
            } else {
                d->hideTip();
            }
        }
        break;
    case QEvent::MouseMove:
        if (obj == d->widget && !d->rect.isNull() && !d->rect.contains(static_cast<QMouseEvent *>(event)->pos())) {
            d->delayHideTip();
        }
        break;
    default:
        break;
    }
    return false;
}
