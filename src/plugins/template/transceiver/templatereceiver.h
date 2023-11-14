// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTRECEIVER_H
#define RECENTRECEIVER_H

#include <framework/framework.h>

class TemplateReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<TemplateReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<TemplateReceiver>;
public:
    explicit TemplateReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
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
