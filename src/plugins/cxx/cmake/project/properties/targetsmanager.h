// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TARGETSMANAGER_H
#define TARGETSMANAGER_H

#include "services/project/projectservice.h"

#include <QObject>

class CMakeCbpParser;
class TargetsManager : public QObject
{
    Q_OBJECT
public:
    static TargetsManager *instance();

    CMakeCbpParser *cbpParser() const;

    void readTargets(const QString &buildDirectory, const QString &workspaceDirectory);

    const QStringList getTargetNamesList() const;
    const QStringList getExeTargetNamesList() const;

    dpfservice::Target getActivedTargetByTargetType(const dpfservice::TargetType type);
    dpfservice::Target getTargetByName(const QString &targetName);

    const dpfservice::Targets getAllTargets() const;

    void updateActivedBuildTarget(const QString &targetName);
    void updateActivedCleanTarget(const QString &targetName);
    void updateActiveExceTarget(const QString &targetName);

    QString getCMakeConfigFile();

signals:
    void initialized(const QString& workspace);

public slots:

private:
    explicit TargetsManager(QObject *parent = nullptr);
    ~TargetsManager();

    bool isGloablTarget(dpfservice::Target &target);
    QString cbpFilePath(const QString &buildFolder);

    dpfservice::Target buildTargetSelected;
    dpfservice::Target cleanTargetSelected;
    dpfservice::Target rebuildTargetSelected;
    dpfservice::Target exeTargetSelected;

    dpfservice::Targets exeTargets;
    dpfservice::Targets targets;

    QStringList buildTargetNameList;
    QStringList exeTargetNameList;

    CMakeCbpParser *parser = nullptr;
};

#endif // TARGETSMANAGER_H
