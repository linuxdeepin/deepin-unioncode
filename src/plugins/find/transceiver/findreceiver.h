// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FINDRECEIVER_H
#define FINDRECEIVER_H

#include <framework/framework.h>

class FindReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<FindReceiver>
{
    Q_OBJECT
public:
    explicit FindReceiver(QObject *parent = nullptr);
    static Type type();
    static QStringList topics();
    virtual void eventProcess(const dpf::Event& event) override;
};

class FindEventTransmit : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FindEventTransmit)
public:
    static FindEventTransmit* instance();

signals:
    void sendProjectPath(const QString &projectPath, const QString &language);
    void sendRemovedProject(const QString &projectPath);
    void sendCurrentEditFile(const QString &filePath, bool actived);

private:
    explicit FindEventTransmit(QObject *parent = nullptr);
    virtual ~FindEventTransmit();
};


#endif // FINDRECEIVER_H
