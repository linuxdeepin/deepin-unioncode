// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionmanager.h"
#include "actionmanager_p.h"
#include "actioncontainer.h"
#include "command_p.h"
#include "util/custompaths.h"

#include <QDir>
#include <QDebug>
#include <QApplication>

constexpr char kKeyboardShortcuts[] = "KeyboardShortcuts";
static ActionManager *m_instance = nullptr;

ActionManagerPrivate::ActionManagerPrivate()
    : settings(CustomPaths::user(CustomPaths::Flags::Configures) + QDir::separator() + QString("shortcut.ini"), QSettings::IniFormat)
{
}

ActionManagerPrivate::~ActionManagerPrivate()
{
    for (auto container : std::as_const(idContainerMap))
        disconnect(container, &QObject::destroyed, this, &ActionManagerPrivate::containerDestroyed);
    qDeleteAll(idContainerMap);
    qDeleteAll(idCmdMap);
}

void ActionManagerPrivate::setContext(const QStringList &context)
{
    cmdContext = context;
    auto it = idCmdMap.cbegin();
    for (; it != idCmdMap.cend(); ++it)
        it.value()->d->setCurrentContext(context);
}

bool ActionManagerPrivate::hasContext(const QStringList &context) const
{
    for (int i = 0; i < cmdContext.size(); ++i) {
        if (context.contains(cmdContext.at(i)))
            return true;
    }
    return false;
}

Command *ActionManagerPrivate::overridableAction(const QString &id)
{
    Command *cmd = idCmdMap.value(id, nullptr);
    if (!cmd) {
        cmd = new Command(id);
        idCmdMap.insert(id, cmd);
        readUserSettings(id, cmd);
        QAction *action = cmd->action();
        mainWindow()->addAction(action);
        action->setObjectName(id);
        action->setShortcutContext(Qt::ApplicationShortcut);
        cmd->d->setCurrentContext(cmdContext);
    }

    return cmd;
}

void ActionManagerPrivate::scheduleContainerUpdate(ActionContainer *actionContainer)
{
    const bool needsSchedule = scheduledContainerUpdates.isEmpty();
    scheduledContainerUpdates.insert(actionContainer);
    if (needsSchedule)
        QMetaObject::invokeMethod(this,
                                  &ActionManagerPrivate::updateContainer,
                                  Qt::QueuedConnection);
}

void ActionManagerPrivate::updateContainer()
{
    for (ActionContainer *c : std::as_const(scheduledContainerUpdates))
        c->update();
    scheduledContainerUpdates.clear();
}

void ActionManagerPrivate::containerDestroyed(QObject *sender)
{
    auto container = static_cast<ActionContainer *>(sender);
    idContainerMap.remove(idContainerMap.key(container));
    scheduledContainerUpdates.remove(container);
}

QWidget *ActionManagerPrivate::mainWindow() const
{
    static QWidget *window { nullptr };
    if (window)
        return window;

    for (auto w : qApp->allWidgets()) {
        if (w->objectName() == "MainWindow") {
            window = w;
            break;
        }
    }

    return window;
}

void ActionManagerPrivate::saveSettings()
{
    auto it = idCmdMap.cbegin();
    for (; it != idCmdMap.cend(); ++it) {
        saveSettings(it.value());
    }
}

void ActionManagerPrivate::saveSettings(Command *cmd)
{
    const auto &id = cmd->id();
    const auto settingsKey = QString(kKeyboardShortcuts) + '/' + id;
    const QList<QKeySequence> keys = cmd->keySequences();
    const QList<QKeySequence> defaultKeys = cmd->defaultKeySequences();
    if (keys != defaultKeys) {
        if (keys.isEmpty()) {
            settings.setValue(settingsKey, QString());
        } else if (keys.size() == 1) {
            settings.setValue(settingsKey, keys.first().toString());
        } else {
            QStringList shortcutList;
            std::transform(keys.begin(), keys.end(), shortcutList.begin(),
                           [](const QKeySequence &k) {
                               return k.toString();
                           });
            settings.setValue(settingsKey, shortcutList);
        }
    } else {
        settings.remove(settingsKey);
    }
}

void ActionManagerPrivate::readUserSettings(const QString &id, Command *cmd)
{
    settings.beginGroup(kKeyboardShortcuts);
    if (settings.contains(id)) {
        const QVariant v = settings.value(id);
        if (QMetaType::Type(v.type()) == QMetaType::QStringList) {
            auto list = v.toStringList();
            QList<QKeySequence> keySequenceList;
            std::transform(list.begin(), list.end(), keySequenceList.begin(),
                           [](const QString &s) {
                               return QKeySequence::fromString(s);
                           });
            cmd->setKeySequences(keySequenceList);
        } else {
            cmd->setKeySequences({ QKeySequence::fromString(v.toString()) });
        }
    }
    settings.endGroup();
}

ActionManager::ActionManager(QObject *parent)
    : QObject(parent),
      d(new ActionManagerPrivate)
{
    m_instance = this;
}

ActionManager::~ActionManager()
{
    delete d;
}

ActionManager *ActionManager::instance()
{
    return m_instance;
}

ActionContainer *ActionManager::createContainer(const QString &containerId)
{
    auto it = d->idContainerMap.constFind(containerId);
    if (it != d->idContainerMap.constEnd())
        return it.value();

    auto mc = new ActionContainer(containerId, d);
    d->idContainerMap.insert(containerId, mc);
    connect(mc, &QObject::destroyed, d, &ActionManagerPrivate::containerDestroyed);

    return mc;
}

ActionContainer *ActionManager::actionContainer(const QString &containerId)
{
    auto it = d->idContainerMap.constFind(containerId);
    if (it == d->idContainerMap.constEnd()) {
        qWarning() << "failed to find :" << containerId;
        return nullptr;
    }
    return it.value();
}

Command *ActionManager::registerAction(QAction *action, const QString &id, const QStringList &context)
{
    Command *cmd = d->overridableAction(id);
    if (cmd) {
        cmd->d->addOverrideAction(action, context);
        emit commandListChanged();
        emit commandAdded(id);
    }
    return cmd;
}

void ActionManager::unregisterAction(QAction *action, const QString &id)
{
    Command *cmd = d->idCmdMap.value(id, nullptr);
    if (!cmd) {
        qWarning() << "unregisterAction: id" << id
                   << "is registered with a different command type.";
        return;
    }

    cmd->d->removeOverrideAction(action);
    if (cmd->d->isEmpty()) {
        // clean up
        d->saveSettings(cmd);
        delete cmd->action();
        d->idCmdMap.remove(id);
        delete cmd;
    }
    emit commandListChanged();
}

Command *ActionManager::command(const QString &id)
{
    auto it = d->idCmdMap.constFind(id);
    if (it == d->idCmdMap.constEnd()) {
        qWarning() << " failed to find :" << id;
        return nullptr;
    }
    return it.value();
}

QList<Command *> ActionManager::commandList()
{
    return d->idCmdMap.values();
}

void ActionManager::addContext(const QStringList &context)
{
    for (const auto &c : context) {
        if (d->cmdContext.contains(c))
            continue;
        d->cmdContext << c;
    }

    setContext(d->cmdContext);
}

void ActionManager::removeContext(const QStringList &context)
{
    for (const auto &c : context) {
        if (!d->cmdContext.contains(c))
            continue;
        d->cmdContext.removeOne(c);
    }

    setContext(d->cmdContext);
}

void ActionManager::saveSettings()
{
    d->saveSettings();
}

void ActionManager::setContext(const QStringList &context)
{
    d->setContext(context);
}

bool ActionManager::hasContext(const QStringList &context)
{
    return d->hasContext(context);
}

QStringList ActionManager::context() const
{
    return d->cmdContext;
}
