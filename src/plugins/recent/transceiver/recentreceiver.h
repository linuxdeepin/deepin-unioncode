// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTRECEIVER_H
#define RECENTRECEIVER_H

#include <framework/framework.h>

class RecentReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<RecentReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<RecentReceiver>;
public:
    explicit RecentReceiver(QObject * parent = nullptr);

    static Type type();
    static QStringList topics();
    virtual void eventProcess(const dpf::Event& event) override;

private:
    void processSaveOpenedProjectEvent(const dpf::Event& event);
    void processSaveOpenedFileEvent(const dpf::Event& event);
    void processSessionCreatedEvent(const dpf::Event& event);
    void processSessionRemovedEvent(const dpf::Event& event);
    void processSessionLoadedEvent(const dpf::Event &event);

private:
    QHash<QString, std::function<void(const dpf::Event &)>> eventHandleMap;
};

class RecentProxy : public QObject
{
    Q_OBJECT
    RecentProxy(){}
    RecentProxy(const RecentProxy&) = delete;

public:
    static RecentProxy* instance();

signals:
    void saveOpenedProject(const QString &kitName,
                           const QString &language,
                           const QString &workspace);
    void saveOpenedFile(const QString &filePath);
};

#endif // RECENTRECEIVER_H
