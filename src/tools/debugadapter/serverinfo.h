// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVERINFO_H
#define SERVERINFO_H

#include <QObject>

extern const int kPort;
class ServerInfo : public QObject
{
    Q_OBJECT
public:
    explicit ServerInfo(QObject *parent = nullptr);

    Q_CLASSINFO("D-Bus Interface", "com.deepin.unioncode.interface")

public slots:
    int port() const;
    void setPort(int port);

    const QString &gethost() const;
    void setHost(QString &_host);

private:
    int iPort = 0;
    QString host;
};

#endif // SERVERINFO_H
