// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionmanager.h"
#include "util/shortcututil.h"
#include "util/custompaths.h"

#include <QAction>
#include <QDir>

static ActionManager *m_instance = nullptr;

class ActionManagerPrivate final
{
public:
    using IdCmdMap = QHash<QString, Action *>;

    ActionManagerPrivate();
    Action *addOverrideAction(QString id, QAction *action);
    Action *removeOverrideAction(QString id);
    Command *command(QString id);
    QList<Command *> commands();

    void addSetting(QString id, Action *action);
    void removeSetting(QString id);
    void saveAllSetting();
    void readUserSetting();

private:
    IdCmdMap idCmdMap;
    QString configFilePath;
};

ActionManagerPrivate::ActionManagerPrivate()
    : configFilePath(CustomPaths::user(CustomPaths::Flags::Configures) + QDir::separator() + QString("shortcut.support"))
{

}

Action *ActionManagerPrivate::addOverrideAction(QString id, QAction *action)
{
    Action *a = idCmdMap.value(id, nullptr);
    if (!a) {
        a = new Action(id, action);
        idCmdMap.insert(id, a);
    }

    return a;
}

Action *ActionManagerPrivate::removeOverrideAction(QString id)
{
    Action *a = idCmdMap.value(id, nullptr);
    if (a) {
        idCmdMap.remove(id);
    }

    return a;
}

Command *ActionManagerPrivate::command(QString id)
{
    return idCmdMap.value(id, nullptr);
}

QList<Command *> ActionManagerPrivate::commands()
{
    QList<Command *> result;
    foreach (Command *cmd, idCmdMap) {
        result << cmd;
    }

    return result;
}

void ActionManagerPrivate::addSetting(QString id, Action *action)
{
    QString shortcut = action->keySequence().toString();
    QString description = action->description();

    QMap<QString, QStringList> shortcutItemMap;
    ShortcutUtil::readFromJson(configFilePath, shortcutItemMap);
    QStringList valueList = {description, shortcut};
    shortcutItemMap[id] = valueList;
    ShortcutUtil::writeToJson(configFilePath, shortcutItemMap);
}

void ActionManagerPrivate::removeSetting(QString id)
{
    QMap<QString, QStringList> shortcutItemMap;
    ShortcutUtil::readFromJson(configFilePath, shortcutItemMap);
    if (shortcutItemMap.contains(id)) {
        shortcutItemMap.remove(id);
    }
    ShortcutUtil::writeToJson(configFilePath, shortcutItemMap);
}

void ActionManagerPrivate::saveAllSetting()
{
    QMap<QString, QStringList> shortcutItemMap;
    IdCmdMap::const_iterator iter = idCmdMap.begin();
    for (; iter != idCmdMap.end(); ++iter)
    {
        QStringList valueList = {iter.value()->description(), iter.value()->keySequence().toString()};
        shortcutItemMap.insert(iter.key(), valueList);
    }

    ShortcutUtil::writeToJson(configFilePath, shortcutItemMap);
}

void ActionManagerPrivate::readUserSetting()
{
    QMap<QString, QStringList> shortcutItemMap;
    ShortcutUtil::readFromJson(configFilePath, shortcutItemMap);

    IdCmdMap::const_iterator iter = idCmdMap.begin();
    for (; iter != idCmdMap.end(); ++iter)
    {
        QString id = iter.key();
        if (shortcutItemMap.contains(id) && iter.value()->action()) {
            QString shortcut = shortcutItemMap.value(id).last();
            iter.value()->action()->setShortcut(QKeySequence(shortcut));
        }
    }
}

ActionManager::ActionManager(QObject *parent)
    : QObject(parent)
    , d(new ActionManagerPrivate())
{

}

ActionManager::~ActionManager()
{
    if (d) {
        delete d;
    }
}

ActionManager *ActionManager::getInstance()
{
    if (!m_instance) {
        m_instance = new ActionManager();
    }
    return m_instance;
}

/*!
    \fn Command *ActionManager::registerAction(QAction *action, const QString id,
                                       const QString description,
                                       const QKeySequence defaultShortcut)

    Makes an action known to the system under the specified action, id, description, default shortcut.
    New a command and insert to map, set the keysequence and description to action, and save info to config file.

    Returns a Command instance that represents the action in the application
    and is owned by the ActionManager.

    Usage: ActionManager::getInstance->registerAction(...);
*/
Command *ActionManager::registerAction(QAction *action, const QString &id,
                                       const QString &description/* = nullptr*/,
                                       const QKeySequence defaultShortcut/* = QKeySequence()*/,
                                       const QString &iconFileName/* = nullptr*/)
{
    if(!action || id.isEmpty())
        return nullptr;

    const QIcon icon = QIcon(iconFileName);
    if (!icon.isNull())
        action->setIcon(icon);
    
    connect(action, &QAction::destroyed, [=] {
        unregisterAction(id);
    });

    Action *a = d->addOverrideAction(id, action);
    if (a) {
        a->setKeySequence(defaultShortcut);
        a->setDescription(description);
    }

    return a;
}

/*!
    \fn void ActionManager::unregisterAction(QString id)

    Removes the knowledge about an action under the specified id.
    Remove from map, set action shortcut to null, and save info to config file.

    Usage: ActionManager::getInstance->unregisterAction(...);
*/
void ActionManager::unregisterAction(QString id)
{
    Action *a = d->removeOverrideAction(id);
    if (a) {
        a->setKeySequence(QKeySequence());
    }
}

Command *ActionManager::command(QString id)
{
    return d->command(id);
}

QList<Command *> ActionManager::commands()
{
    return d->commands();
}

void ActionManager::readUserSetting()
{
    return d->readUserSetting();
}

void ActionManager::saveSetting()
{
    return d->saveAllSetting();
}

