// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "consolemanager.h"

#include <QVBoxLayout>

class ConsoleManagerPrivate
{
public:
    friend class ConsoleManager;

    QVBoxLayout *mainLayout { nullptr };
    ConsoleWidget *currentConsole { nullptr };
    QList<ConsoleWidget *> consoleList;
};

ConsoleManager::ConsoleManager(QWidget *parent)
    : QWidget(parent), d(new ConsoleManagerPrivate)
{
    d->mainLayout = new QVBoxLayout(this);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
}

ConsoleManager::~ConsoleManager()
{
    delete d;
}

QTermWidget *ConsoleManager::getCurrentConsole()
{
    //todo: when switch to other project`s path, open a new console
    return d->currentConsole;
}

void ConsoleManager::executeCommand(const QString &text)
{
    if (!d->currentConsole)
        initDefaultConsole();
    d->currentConsole->sendText(text);
}

void ConsoleManager::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    if (!d->currentConsole)
        initDefaultConsole();
}

void ConsoleManager::initDefaultConsole()
{
    auto defaultConsole = new ConsoleWidget(this);
    d->consoleList.append(defaultConsole);
    d->currentConsole = defaultConsole;

    d->mainLayout->addWidget(defaultConsole);
}
