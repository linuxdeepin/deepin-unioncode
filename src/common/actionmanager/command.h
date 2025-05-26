// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QAction;
class QKeySequence;
class QToolButton;
QT_END_NAMESPACE

class CommandPrivate;
class Command : public QObject
{
    Q_OBJECT
public:
    enum CommandAttribute {
        CA_Hide = 1,
        CA_UpdateText = 2,
        CA_UpdateIcon = 4,
        CA_NonConfigurable = 8
    };
    Q_DECLARE_FLAGS(CommandAttributes, CommandAttribute)

    Command(const QString &id, QObject *parent = nullptr);
    ~Command();

    void setDefaultKeySequence(const QKeySequence &key);
    void setDefaultKeySequences(const QList<QKeySequence> &keys);
    void setKeySequences(const QList<QKeySequence> &keys);
    QList<QKeySequence> defaultKeySequences() const;
    QList<QKeySequence> keySequences() const;
    QKeySequence keySequence() const;

    void setDescription(const QString &text);
    QString description() const;
    QString id() const;
    QAction *action() const;
    QStringList context() const;

    void setAttribute(CommandAttribute attr);
    void removeAttribute(CommandAttribute attr);
    bool hasAttribute(CommandAttribute attr) const;

    bool isActive() const;
    static QStringList keySequencesToNativeString(const QList<QKeySequence> &sequences);

signals:
    void keySequenceChanged();
    void activeStateChanged();

private:
    friend class ActionManager;
    friend class ActionManagerPrivate;

    CommandPrivate *const d;
};

#endif   // COMMAND_H
