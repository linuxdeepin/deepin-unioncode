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
#ifndef POLKIT_H
#define POLKIT_H

#include <QString>
#include <QObject>

class PolKit final: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PolKit)
    PolKit();
public:
    PolKit &instance();

    // Asynchronous
    qint64 execute(const QString & program, const QStringList &arguments);

    // kill Polkit
    void cancel(qint64 executeID);

signals:
    void succeeded();
    void failed(const QByteArray &);
    void canceled();
};

#endif // POLKIT_H
