// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dependencemanager.h"
#include "notify/notificationmanager.h"
#include "uicontroller/controller.h"

#include "base/baseitemdelegate.h"

#include <DDialog>
#include <DLabel>

#include <QListView>
#include <QStringListModel>
#include <QMap>
#include <QTimer>
#include <QDebug>
#include <QMutexLocker>
#include <QtConcurrent>
#include <QVBoxLayout>

#include <functional>

const char CancelActionID[] { "cancel" };
const char ViewDetails[] { "view_details" };
const char InstallActionID[] { "install_default" };

DWIDGET_USE_NAMESPACE

class DependenceManagerPrivate : public QObject
{
public:
    void init();
    void notify();
    void handleActionInvoke(const QString &actId, const InstallInfo &info);
    void install(const InstallInfo &info);
    void showDetails(const InstallInfo &info);
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
                             ViewDetails, DependenceManager::tr("View Details"),
                             InstallActionID, DependenceManager::tr("Install") };

    QMutexLocker lk(&mutex);
    while (!infoList.isEmpty()) {
        const auto &info = infoList.takeFirst();
        auto cb = std::bind(&DependenceManagerPrivate::handleActionInvoke, this, _1, info);
        QString msg = DependenceManager::tr("Request to install some dependency packages. "
                                            "Do you want to install them?");
        NotificationManager::instance()->notify(0, info.plugin, msg, actionList, cb);
    }
}

void DependenceManagerPrivate::handleActionInvoke(const QString &actId, const InstallInfo &info)
{
    if (actId == InstallActionID)
        install(info);
    else if (actId == ViewDetails)
        showDetails(info);
}

void DependenceManagerPrivate::install(const InstallInfo &info)
{
    if (!installerMap.contains(info.installer))
        return;

    installerMap[info.installer]->install(info);
}

void DependenceManagerPrivate::showDetails(const InstallInfo &info)
{
    DDialog dlg(Controller::instance()->mainWindow());
    dlg.setIcon(QIcon::fromTheme("ide"));
    dlg.setWindowTitle(DependenceManager::tr("Installation Details"));
    dlg.addButton(DependenceManager::tr("Cancel", "button"));
    dlg.addButton(DependenceManager::tr("Install", "button"), true, DDialog::ButtonRecommend);

    QWidget *widget = new QWidget(&dlg);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    DLabel *label = new DLabel(DependenceManager::tr("Installation package list:"));
    label->setWordWrap(true);

    QListView *view = new QListView(&dlg);
    QStringListModel *model = new QStringListModel(view);
    model->setStringList(info.packageList);
    view->setModel(model);
    view->setEditTriggers(QListView::NoEditTriggers);
    view->setFrameShape(QFrame::NoFrame);
    view->setItemDelegate(new BaseItemDelegate(view));
    view->setSizeAdjustPolicy(QListView::AdjustToContents);

    layout->addWidget(label);
    layout->addWidget(view, 1);
    dlg.addContent(widget);
    int code = dlg.exec();
    if (code == 1)
        install(info);
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

    QtConcurrent::run([this, plugin, name, packageList]() {
        return d->checkInstalled(plugin, name, packageList);
    });
    return true;
}

QMap<QString, AbstractInstaller *> DependenceManager::allInstaller() const
{
    return d->installerMap;
}
