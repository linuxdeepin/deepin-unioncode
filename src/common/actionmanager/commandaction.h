// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDACTION_H
#define COMMANDACTION_H

#include <QPointer>
#include <QAction>

class CommandActionPrivate;
class CommandAction : public QAction
{
    Q_OBJECT
public:
    enum Attribute {
        Hide = 0x01,
        UpdateText = 0x02,
        UpdateIcon = 0x04
    };
    Q_DECLARE_FLAGS(Attributes, Attribute)

    explicit CommandAction(QObject *parent = nullptr);
    ~CommandAction();

    void initialize(QAction *action);

    void setAction(QAction *action);
    QAction *action() const;

    bool shortcutVisibleInToolTip() const;
    void setShortcutVisibleInToolTip(bool visible);

    void setAttribute(Attribute attribute);
    void removeAttribute(Attribute attribute);
    bool hasAttribute(Attribute attribute);

    static QString stringWithAppendedShortcut(const QString &str, const QKeySequence &shortcut);
    static CommandAction *commandActionWithIcon(QAction *original, const QIcon &newIcon);

signals:
    void currentActionChanged(QAction *action);

private:
    CommandActionPrivate *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CommandAction::Attributes)

#endif   // COMMANDACTION_H
