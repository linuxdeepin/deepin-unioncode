// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QRect>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QLayout;
class QWidget;
QT_END_NAMESPACE

class ToolTipPrivate;
class ToolTip : public QObject
{
    Q_OBJECT
public:
    static ToolTip *instance();

    static void show(const QPoint &pos, const QString &content, QWidget *w = nullptr, const QRect &rect = QRect());
    static void show(const QPoint &pos, QWidget *content, QWidget *w = nullptr, const QRect &rect = QRect());
    static void show(const QPoint &pos, QLayout *content, QWidget *w = nullptr, const QRect &rect = QRect());
    static void hide();
    static void hideImmediately();
    static bool isVisible();

Q_SIGNALS:
    void hidden();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    ToolTip();
    ~ToolTip();

    ToolTipPrivate *const d;
};

#endif   // TOOLTIP_H
