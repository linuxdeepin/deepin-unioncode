// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTINSTALLER_H
#define ABSTRACTINSTALLER_H

#include <QObject>

struct InstallInfo
{
    QString installer;
    QString plugin;
    QStringList packageList;
};

class AbstractInstaller : public QObject
{
    Q_OBJECT
public:
    explicit AbstractInstaller(QObject *parent = nullptr);

    virtual QString description() = 0;
    virtual bool checkInstalled(const QString &package) = 0;
    virtual void install(const InstallInfo &info) = 0;
};

#endif   // ABSTRACTINSTALLER_H
