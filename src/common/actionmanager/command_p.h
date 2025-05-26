// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMAND_P_H
#define COMMAND_P_H

#include "command.h"
#include "commandaction.h"

#include <QList>
#include <QPointer>
#include <QMap>
#include <QKeySequence>

class CommandPrivate : public QObject
{
public:
    CommandPrivate(const QString &id, Command *qq);

    void setCurrentContext(const QStringList &context);
    void addOverrideAction(QAction *action, const QStringList &context);
    void removeOverrideAction(QAction *action);

    bool isEmpty() const;
    void updateActiveState();
    void setActive(bool state);

public:
    Command *q;

    QString cmdId;
    Command::CommandAttributes attributes;
    QList<QKeySequence> defaultKeys;
    QString defaultText;
    bool isKeyInitialized { false };

    QStringList cmdContext;
    CommandAction *action { nullptr };
    QMap<QString, QPointer<QAction>> contextActionMap;
    QString toolTip;
    bool active { false };
};

#endif // COMMAND_P_H
