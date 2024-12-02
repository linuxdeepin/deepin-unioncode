// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STATUSINFOMANAGER_H
#define STATUSINFOMANAGER_H

#include <QObject>

namespace dpfservice {
class WindowService;
}

class StatusInfoManagerPrivate;
class StatusInfoManager : public QObject
{
    Q_OBJECT
public:
    static StatusInfoManager *instance();

    void init(dpfservice::WindowService *winSrv);

private:
    explicit StatusInfoManager(QObject *parent = nullptr);
    ~StatusInfoManager();

    StatusInfoManagerPrivate *const d;
};

#endif   // STATUSINFOMANAGER_H
