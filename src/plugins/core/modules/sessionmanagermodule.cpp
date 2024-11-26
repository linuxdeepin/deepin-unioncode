// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionmanagermodule.h"
#include "session/sessionmanager.h"

#include "services/session/sessionservice.h"
#include "common/util/eventdefinitions.h"

using namespace dpfservice;

void SessionManagerModule::initialize(Controller *_uiController)
{
    connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted, this, [this] {
        auto ins = SessionManager::instance();
        if (ins->isAutoLoadLastSession()) {
            const auto session = ins->lastSession();
            QTimer::singleShot(100, this, std::bind(&SessionManager::loadSession, ins, session));
        }
    });
    initInterfaces();
    initOutputEvents();
}

void SessionManagerModule::initInterfaces()
{
    auto sessionSrv = dpfGetService(SessionService);
    Q_ASSERT(sessionSrv);

    using namespace std::placeholders;
    auto ins = SessionManager::instance();
    sessionSrv->currentSession = std::bind(&SessionManager::currentSession, ins);
    sessionSrv->lastSession = std::bind(&SessionManager::lastSession, ins);
    sessionSrv->sessionList = std::bind(&SessionManager::sessionList, ins);
    sessionSrv->sessionDateTime = std::bind(&SessionManager::sessionDateTime, ins, _1);
    sessionSrv->lastActiveTime = std::bind(&SessionManager::lastActiveTime, ins, _1);
    sessionSrv->createSession = std::bind(&SessionManager::createSession, ins, _1);
    sessionSrv->removeSession = std::bind(&SessionManager::removeSession, ins, _1);
    sessionSrv->removeSessions = std::bind(&SessionManager::removeSessions, ins, _1);
    sessionSrv->renameSession = std::bind(&SessionManager::renameSession, ins, _1, _2);
    sessionSrv->cloneSession = std::bind(&SessionManager::cloneSession, ins, _1, _2);
    sessionSrv->showSessionManager = std::bind(&SessionManager::showSessionManager, ins);
    sessionSrv->setValue = std::bind(&SessionManager::setValue, ins, _1, _2);
    sessionSrv->value = std::bind(&SessionManager::value, ins, _1);
    sessionSrv->loadSession = std::bind(&SessionManager::loadSession, ins, _1);
    sessionSrv->saveSession = std::bind(&SessionManager::saveSession, ins);
    sessionSrv->isDefaultSession = std::bind(&SessionManager::isDefaultSession, ins, _1);
    sessionSrv->isSessionLoading = std::bind(&SessionManager::isSessionLoading, ins);
    sessionSrv->isDefaultVirgin = std::bind(&SessionManager::isDefaultVirgin, ins);
    sessionSrv->markSessionFileDirty = std::bind(&SessionManager::markSessionFileDirty, ins);
    sessionSrv->sessionFile = std::bind(&SessionManager::sessionFile, ins, _1);
}

void SessionManagerModule::initOutputEvents()
{
    auto ins = SessionManager::instance();
    connect(ins, &SessionManager::readyToSaveSession, this,
            [] {
                session.readyToSaveSession();
            });
    connect(ins, &SessionManager::sessionLoaded, this,
            [](const QString &name) {
                session.sessionLoaded(name);
            });
    connect(ins, &SessionManager::sessionCreated, this,
            [](const QString &name) {
                session.sessionCreated(name);
            });
    connect(ins, &SessionManager::sessionRenamed, this,
            [](const QString &oldName, const QString &newName) {
                session.sessionRenamed(oldName, newName);
            });
    connect(ins, &SessionManager::sessionRemoved, this,
            [](const QString &name) {
                session.sessionRemoved(name);
            });
    connect(ins, &SessionManager::sessionStatusChanged, this,
            [] {
                session.sessionStatusChanged();
            });
}
