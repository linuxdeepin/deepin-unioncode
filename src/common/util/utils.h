// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <DToolButton>

#include <QAction>

DWIDGET_USE_NAMESPACE
namespace utils {
    
    static DToolButton* createIconButton(QAction *action, QWidget *parent) {
        DToolButton *iconBtn = new DToolButton(parent);
        iconBtn->setFocusPolicy(Qt::NoFocus);
        iconBtn->setEnabled(action->isEnabled());
        iconBtn->setIcon(action->icon());
        iconBtn->setFixedSize(QSize(36, 36));

        QString toolTipStr = action->text();
        if (!action->shortcut().isEmpty()) {
            toolTipStr = toolTipStr + " " + action->shortcut().toString();
            iconBtn->setShortcut(action->shortcut());
        }

        if (!toolTipStr.isEmpty())
            iconBtn->setToolTip(toolTipStr);

        QObject::connect(iconBtn, &DToolButton::clicked, action, &QAction::triggered);
        QObject::connect(action, &QAction::changed, iconBtn, [=] {
            QString toolTipStr = action->text() + " " + action->shortcut().toString();
            iconBtn->setToolTip(toolTipStr);
            iconBtn->setShortcut(action->shortcut());

            iconBtn->setIcon(action->icon());
            iconBtn->setEnabled(action->isEnabled());
        });

        return iconBtn;
    }
    
}

#endif
