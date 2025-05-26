// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHORTCUTITEM_H
#define SHORTCUTITEM_H

#include "common/actionmanager/command.h"

#include <DFrame>
#include <DLabel>

#include <QTreeWidgetItem>

struct ShortcutItem
{
    Command *cmd { nullptr };
    QList<QKeySequence> shortcutKeys;
    QTreeWidgetItem *item { nullptr };
};

class KeyLabel : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    explicit KeyLabel(QWidget *parent = nullptr);

    void setKeySquece(const QString &keySequece);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawShadow(QPainter *p, const QRect &rect, const QColor &color) const;

private:
    DTK_WIDGET_NAMESPACE::DLabel *label { nullptr };
};

class ShortcutLabel : public QWidget
{
    Q_OBJECT
public:
    explicit ShortcutLabel(QWidget *parent = nullptr);

    void setKeySqueces(const QStringList &keySequeces);
};

Q_DECLARE_METATYPE(ShortcutItem *)
#endif   // SHORTCUTITEM_H
