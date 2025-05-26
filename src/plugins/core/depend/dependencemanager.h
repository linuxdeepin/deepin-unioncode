// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEPENDENCEMANAGER_H
#define DEPENDENCEMANAGER_H

#include "base/abstractinstaller.h"

#include <QObject>

class DependenceManagerPrivate;
class DependenceManager : public QObject
{
    Q_OBJECT
public:
    explicit DependenceManager(QObject *parent = nullptr);
    ~DependenceManager();

    void setNotifiable(bool notifiable);

    void registerInstaller(const QString &name, AbstractInstaller *installer);
    bool installPackageList(const QString &plugin, const QString &name, const QStringList &packageList, QString *error = nullptr);
    QMap<QString, AbstractInstaller *> allInstaller() const;

private:
    DependenceManagerPrivate *const d;
};

#endif // DEPENDENCEMANAGER_H
