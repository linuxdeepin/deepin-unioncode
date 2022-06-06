/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#ifndef HANDLER_H
#define HANDLER_H

#include <QObject>
#include <QProcess>
#include <QTcpSocket>

class Handler : public QObject
{
    Q_OBJECT
public:
    Handler();
    virtual ~Handler();
    virtual void bind(QProcess *qIODevice);
    virtual void bind(QTcpSocket *qIODevice);
    virtual void filterData(const QByteArray &array) {Q_UNUSED(array)}
    QIODevice *getDevice() { return device;}

signals:
    void nowReadedAll(const QByteArray &array);

public slots:
    void nowToWrite(const QByteArray &array);

private slots:
    void doReadAll();

protected:
    QIODevice *device;
};

#endif // HANDLER_H
