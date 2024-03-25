// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include <QKeySequence>

class QAction;
class QKeySequence;
class ActionPrivate;
class Command : public QObject
{
    Q_OBJECT
public:
    explicit Command(QObject *parent = nullptr)
        : QObject(parent) {}

    virtual QString id() const = 0;
    virtual QAction *action() const = 0;

    virtual void setKeySequence(const QKeySequence &key) = 0;
    virtual QKeySequence keySequence() const = 0;

    virtual void setDescription(const QString &text) = 0;
    virtual QString description() const = 0;

signals:
    void keySequenceChanged();
};

class Action : public Command
{
    Q_OBJECT
public:
    Action(QString id, QAction *action, QObject *parent = nullptr);
    virtual ~Action() override;

    QString id() const override;
    QAction *action() const override;

    void setKeySequence(const QKeySequence &key) override;
    QKeySequence keySequence() const override;

    void setDescription(const QString &text) override;
    QString description() const override;

private:
    ActionPrivate *const d;
};

#endif   // COMMAND_H
