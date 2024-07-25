// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
