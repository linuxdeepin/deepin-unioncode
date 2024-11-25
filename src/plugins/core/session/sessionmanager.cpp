// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionmanager.h"
#include "sessiondialog.h"
#include "uicontroller/controller.h"

#include "common/settings/settings.h"
#include "common/util/custompaths.h"

#include <QDir>
#include <QDateTime>

constexpr char kDefaultSession[] { "default" };
constexpr char kSessionGroup[] { "Session" };
constexpr char kGeneralGroup[] { "General" };

constexpr char kLastActiveTimes[] { "LastActiveTimes" };
constexpr char kIsAutoLoadLastSession[] { "AutoLoadLastSession" };
constexpr char kLastSession[] { "LastSession" };

class SessionManagerPrivate
{
public:
    explicit SessionManagerPrivate(SessionManager *qq);

    QString sessionConfigDir();
    QString settingFile();
    void readSettings();
    void saveSettings();
    void restoreValues(const QString &session);
    void setAutoLoadLastSession(bool autoLoad);

public:
    SessionManager *q;

    QString currentSession { kDefaultSession };
    bool isSessionLoading { false };
    bool isAutoLoad { false };
    bool virginSession { true };

    QStringList sessionList;
    QMap<QString, QVariant> values;
    QHash<QString, QDateTime> sessionDateTimes;
    QHash<QString, QDateTime> lastActiveTimes;

    Settings settings;
};

SessionManagerPrivate::SessionManagerPrivate(SessionManager *qq)
    : q(qq)
{
}

QString SessionManagerPrivate::sessionConfigDir()
{
    const QString configDir = CustomPaths::user(CustomPaths::Flags::Configures);
    return configDir + QDir::separator() + "sessions";
}

QString SessionManagerPrivate::settingFile()
{
    return sessionConfigDir() + QDir::separator() + "session.json";
}

void SessionManagerPrivate::readSettings()
{
    settings.load("", settingFile());
    isAutoLoad = settings.value(kGeneralGroup, kIsAutoLoadLastSession).toBool();
}

void SessionManagerPrivate::saveSettings()
{
    QVariantMap times;
    for (auto it = lastActiveTimes.cbegin(); it != lastActiveTimes.cend(); ++it) {
        times.insert(it.key(), it.value());
    }
    settings.setValue(kGeneralGroup, kLastActiveTimes, times);
    settings.setValue(kGeneralGroup, kLastSession, currentSession);
    settings.setValue(kGeneralGroup, kIsAutoLoadLastSession, isAutoLoad);
}

void SessionManagerPrivate::restoreValues(const QString &session)
{
    Settings settings("", q->sessionFile(session));
    const auto keys = settings.keyList(kSessionGroup);
    for (const auto &key : keys) {
        values.insert(key, settings.value(kSessionGroup, key));
    }
}

void SessionManagerPrivate::setAutoLoadLastSession(bool autoLoad)
{
    if (isAutoLoad == autoLoad)
        return;
    isAutoLoad = autoLoad;
    settings.setValue(kGeneralGroup, kIsAutoLoadLastSession, autoLoad);
    settings.sync();
}

SessionManager::SessionManager(QObject *parent)
    : QObject(parent),
      d(new SessionManagerPrivate(this))
{
    d->readSettings();
    connect(qApp, &QApplication::aboutToQuit, this, [this] {
        d->saveSettings();
        saveSession();
    });
}

SessionManager::~SessionManager()
{
    delete d;
}

SessionManager *SessionManager::instance()
{
    static SessionManager ins;
    return &ins;
}

QString SessionManager::currentSession()
{
    return d->currentSession;
}

QString SessionManager::lastSession()
{
    return d->settings.value(kGeneralGroup, kLastSession).toString();
}

QStringList SessionManager::sessionList()
{
    if (d->sessionList.isEmpty()) {
        QDir sessionDir(d->sessionConfigDir());
        const auto &sessionInfos = sessionDir.entryInfoList({ "*.session" }, QDir::NoDotAndDotDot | QDir::Files,
                                                            QDir::Time | QDir::Reversed);
        const QVariantMap lastActiveTimes = d->settings.value(kGeneralGroup, kLastActiveTimes).toMap();
        for (const auto &info : sessionInfos) {
            const auto &name = info.baseName();
            d->sessionDateTimes.insert(name, info.lastModified());
            const auto lastActiveTime = lastActiveTimes.find(name);
            // clang-format off
            d->lastActiveTimes.insert(name, lastActiveTime != lastActiveTimes.end()
                                            ? lastActiveTime->toDateTime()
                                            : info.lastModified());
            // clang-format on
            if (info.baseName() != kDefaultSession)
                d->sessionList << info.baseName();
        }
        d->sessionList.prepend(kDefaultSession);
    }
    return d->sessionList;
}

QDateTime SessionManager::sessionDateTime(const QString &session)
{
    return d->sessionDateTimes.value(session);
}

QDateTime SessionManager::lastActiveTime(const QString &session)
{
    return d->lastActiveTimes.value(session);
}

bool SessionManager::createSession(const QString &session)
{
    if (sessionList().contains(session))
        return false;

    Q_ASSERT(d->sessionList.size() > 0);
    d->sessionList.insert(1, session);
    d->lastActiveTimes.insert(session, QDateTime::currentDateTime());
    Q_EMIT sessionCreated(session);
    return true;
}

bool SessionManager::removeSession(const QString &session)
{
    if (!d->sessionList.contains(session))
        return false;

    d->sessionList.removeOne(session);
    d->lastActiveTimes.remove(session);
    Q_EMIT sessionRemoved(session);

    const auto &file = sessionFile(session);
    if (QFile::exists(file))
        return QFile::remove(file);
    return false;
}

void SessionManager::removeSessions(const QStringList &sessions)
{
    for (const auto &session : sessions)
        removeSession(session);
}

bool SessionManager::renameSession(const QString &oldName, const QString &newName)
{
    if (!cloneSession(oldName, newName))
        return false;
    if (oldName == currentSession())
        loadSession(newName);
    Q_EMIT sessionRenamed(oldName, newName);
    return removeSession(oldName);
}

bool SessionManager::cloneSession(const QString &select, const QString &clone)
{
    if (!d->sessionList.contains(select))
        return false;

    QFile file(sessionFile(select));
    if (!file.exists() || file.copy(sessionFile(clone))) {
        d->sessionList.insert(1, clone);
        d->sessionDateTimes.insert(clone, QFileInfo(sessionFile(clone)).lastModified());
        Q_EMIT sessionCreated(clone);
        return true;
    }
    return false;
}

void SessionManager::showSessionManager()
{
    saveSession();
    SessionDialog dlg(Controller::instance()->mainWindow());
    dlg.setAutoLoadSession(d->isAutoLoad);
    dlg.exec();
    d->setAutoLoadLastSession(dlg.autoLoadSession());
}

void SessionManager::setValue(const QString &key, const QVariant &value)
{
    if (d->values.value(key) == value)
        return;
    d->values.insert(key, value);
}

QVariant SessionManager::value(const QString &key)
{
    return d->values.value(key, QVariant());
}

bool SessionManager::loadSession(const QString &session)
{
    if (session == d->currentSession && !isDefaultVirgin())
        return true;

    bool isEmptySession = session.isEmpty();
    if (!isEmptySession && !sessionList().contains(session))
        return false;

    const auto &cfgFile = sessionFile(isEmptySession ? kDefaultSession : session);
    if (QFile::exists(cfgFile) && isEmptySession) {
        d->restoreValues(kDefaultSession);
        Q_EMIT sessionLoaded(kDefaultSession);
        return true;
    } else if (isEmptySession) {
        Q_EMIT sessionLoaded(kDefaultSession);
        return true;
    }

    d->isSessionLoading = true;
    if (!isDefaultVirgin())
        saveSession();

    d->virginSession = false;
    d->values.clear();
    d->currentSession = session;
    if (QFile::exists(cfgFile))
        d->restoreValues(session);
    d->lastActiveTimes.insert(session, QDateTime::currentDateTime());
    Q_EMIT sessionLoaded(session);

    d->isSessionLoading = false;
    return true;
}

bool SessionManager::saveSession()
{
    Q_EMIT readyToSaveSession();

    const auto &cfgFile = sessionFile(d->currentSession);
    Settings settings("", cfgFile);
    auto iter = d->values.cbegin();
    for (; iter != d->values.cend(); ++iter) {
        settings.setValue(kSessionGroup, iter.key(), iter.value());
    }

    return true;
}

bool SessionManager::isDefaultSession(const QString &session)
{
    return session == kDefaultSession;
}

bool SessionManager::isSessionLoading()
{
    return d->isSessionLoading;
}

bool SessionManager::isDefaultVirgin()
{
    return isDefaultSession(d->currentSession) && d->virginSession;
}

bool SessionManager::isAutoLoadLastSession()
{
    return d->isAutoLoad;
}

QString SessionManager::sessionFile(const QString &session)
{
    QString format = "%1/%2.session";
    return format.arg(d->sessionConfigDir(), session);
}
