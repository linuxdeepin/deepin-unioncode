// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NINJABUILD_H
#define NINJABUILD_H

#include "dap/protocol.h"
#include <QObject>

class NinjaBuildPrivate;
class NinjaBuild : public QObject
{
    Q_OBJECT
public:
    explicit NinjaBuild(QObject *parent = nullptr);
    ~NinjaBuild();

    static QString build(const QString& kitName, const QString& projectPath);

signals:

private slots:

private:
    NinjaBuildPrivate *const d;
};

#endif //NINJABUILD_H

