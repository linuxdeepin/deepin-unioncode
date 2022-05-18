/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
    Action(QString id, QAction *action);
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

#endif // COMMAND_H
