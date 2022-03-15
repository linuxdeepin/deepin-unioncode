/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
    int iPort = kPort;
    QString host;
};

#endif // SERVERINFO_H
