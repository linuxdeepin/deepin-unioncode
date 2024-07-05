// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dependencemanager.h"
#include "notify/notificationmanager.h"

#include <QMap>
#include <QTimer>
#include <QDebug>
#include <QMutexLocker>
#include <QtConcurrent>

#include <functional>

const char CancelActionID[] { "cancel" };
const char InstallActionID[] { "install_default" };

class DependenceManagerPrivate : public QObject
{
public:
    void init();
    void notify();
    void installHander(const QString &actId, const InstallInfo &info);
    void checkInstalled(const QString &plugin, const QString &name, const QStringList &packageList);

public:
    QMutex mutex;
    QMap<QString, AbstractInstaller *> installerMap;
    QList<InstallInfo> infoList;
    QTimer notifyTimer;
    bool notifiable { false };
};

void DependenceManagerPrivate::init()
{
    notifyTimer.setSingleShot(true);
    notifyTimer.setInterval(500);

    connect(&notifyTimer, &QTimer::timeout, this, &DependenceManagerPrivate::notify);
}

void DependenceManagerPrivate::notify()
{
    using namespace std::placeholders;
    if (!notifiable)
        notifyTimer.start();

    QStringList actionList { CancelActionID, DependenceManager::tr("Cancel"),
                             InstallActionID, DependenceManager::tr("Install") };

    QMutexLocker lk(&mutex);
    while (!infoList.isEmpty()) {
        const auto &info = infoList.takeFirst();
        auto cb = std::bind(&DependenceManagerPrivate::installHander, this, _1, info);
        QString msg = DependenceManager::tr("Request to install some dependency packages. "
                                            "Do you want to install them?");
        NotificationManager::instance()->notify(0, info.plugin, msg, actionList, cb);
    }
}

void DependenceManagerPrivate::installHander(const QString &actId, const InstallInfo &info)
{
    if (actId != InstallActionID)
        return;

    if (!installerMap.contains(info.installer))
        return;

    installerMap[info.installer]->install(info);
}

void DependenceManagerPrivate::checkInstalled(const QString &plugin, const QString &name, const QStringList &packageList)
{
    QStringList list;
    for (const auto &package : packageList) {
        if (!installerMap[name]->checkInstalled(package))
            list << package;
    }

    if (list.isEmpty())
        return;

    InstallInfo info;
    info.installer = name;
    info.plugin = plugin;
    info.packageList = list;

    QMutexLocker lk(&mutex);
    infoList << info;
    lk.unlock();

    QMetaObject::invokeMethod(&notifyTimer, qOverload<>(&QTimer::start));
}

DependenceManager::DependenceManager(QObject *parent)
    : QObject(parent),
      d(new DependenceManagerPrivate)
{
    d->init();
}

DependenceManager::~DependenceManager()
{
    delete d;
}

void DependenceManager::setNotifiable(bool notifiable)
{
    d->notifiable = notifiable;
}

void DependenceManager::registerInstaller(const QString &name, AbstractInstaller *installer)
{
    if (d->installerMap.contains(name)) {
        QString msg("The installer \"%1\" has been registered.");
        qWarning() << msg.arg(name);
        return;
    }

    d->installerMap.insert(name, installer);
}

bool DependenceManager::installPackageList(const QString &plugin, const QString &name, const QStringList &packageList, QString *error)
{
    if (!d->installerMap.contains(name)) {
        QString msg("The installer \"%1\" has not been registered.");
        if (error)
            *error = msg.arg(name);
        qWarning() << msg.arg(name);
        return false;
    }

    QtConcurrent::run(d, &DependenceManagerPrivate::checkInstalled, plugin, name, packageList);
    return true;
}

QMap<QString, AbstractInstaller *> DependenceManager::allInstaller() const
{
    return d->installerMap;
}
