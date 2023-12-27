// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTINFO_H
#define PROJECTINFO_H

#include "projectactioninfo.h"

#include <QSet>
#include <QVariantHash>
#include <QStandardItem>

namespace dpfservice {

enum
{
    ProjectInfoRole = Qt::ItemDataRole::UserRole,
};

class ProjectInfo
{
public:
    ProjectInfo() : data({}) {}
    ProjectInfo(const ProjectInfo &other) {data = other.data;}

    bool operator == (const ProjectInfo &other) const {return data == other.data;}
    bool isEmpty() const {return data.isEmpty();}
    inline bool hasKey(const QString &key) {return data.keys().contains(key);}

    inline void setProperty(const QString &key, QVariant value) {data[key] = value;}
    inline QVariant property(const QString &key) const {return data[key];}

    inline void setLanguage(const QString &language) {data["Language"] = language;}
    inline void setKitName(const QString &kitName) {data["KitName"] = kitName;}
    inline void setBuildFolder(const QString &buildPath) {data["BuildFolder"] = buildPath;}
    inline void setWorkspaceFolder(const QString &workspaceFolder) {data["WorkspaceFolder"] = workspaceFolder;}
    inline void setBuildType(const QString &buildType) {data["BuildType"] = buildType;}
    inline void setBuildProgram(const QString &program) {data["BuildProgram"] = program;}
    inline void setConfigCustomArgs(const QStringList &args) {data["ConfigCustomArgs"] = args;}
    inline void setSourceFiles(const QSet<QString> &files) {data["SourceFiles"] = QVariant::fromValue(files);}
    inline void setDetailInformation(bool isDetail) {data["DetailInformation"] = isDetail;}

    inline void setBuildCustomArgs(const QStringList &args) {data["BuildCustomArgs"] = args;}
    inline void setCleanCustomArgs(const QStringList &args) {data["CleanCustomArgs"] = args;}
    inline void setRunProgram(const QString &program) {data["RunProgram"] = program;}
    inline void setRunCustomArgs(const QStringList &args) {data["RunCustomArgs"] = args;}
    inline void setRunWorkspaceDir(const QString &workspaceDir) {data["RunWorkspaceDir"] = workspaceDir;}
    inline void setRunEnvironment(const QStringList &envs) {data["RunEnvironment"] = envs;}

    inline QString language() const {return data["Language"].toString();}
    inline QString kitName() const {return data["KitName"].toString();}
    inline QString buildFolder() const {return data["BuildFolder"].toString();}
    inline QString workspaceFolder() const {return data["WorkspaceFolder"].toString();}
    inline QString buildType() const {return data["BuildType"].toString();}
    inline QString buildProgram() const {return data["BuildProgram"].toString();}
    inline QStringList configCustomArgs() const {return data["ConfigCustomArgs"].toStringList();}
    inline QSet<QString> sourceFiles() const {return qvariant_cast<QSet<QString>>(data["SourceFiles"]);}
    inline bool detailInformation() const {return data["DetailInformation"].toBool();}

    inline QStringList buildCustomArgs() const {return data["BuildCustomArgs"].toStringList();}
    inline QStringList cleanCustomArgs() const {return data["CleanCustomArgs"].toStringList();}
    inline QString runProgram() const {return data["RunProgram"].toString();}
    inline QStringList runCustomArgs() const {return data["RunCustomArgs"].toStringList();}
    inline QString runWorkspaceDir() const {return data["RunWorkspaceDir"].toString();}
    inline QStringList runEnvironment() const {return data["RunEnvironment"].toStringList();}

    inline static void set(QStandardItem *root, const ProjectInfo &info)
    {
        if (root)
            root->setData(QVariant::fromValue(info), ProjectInfoRole);
    }

    inline static ProjectInfo get(const QModelIndex &root)
    {
        if (root.isValid())
            return qvariant_cast<ProjectInfo>(root.data(ProjectInfoRole));
        return {};
    }

    inline static ProjectInfo get(const QStandardItem *root)
    {
        if (root)
            return qvariant_cast<ProjectInfo>(root->data(ProjectInfoRole));
        return {};
    }

    inline static void set(QStandardItem *any, const QString &key, const QVariant &value)
    {
        if (any) {
            auto info = get(any);
            info.setProperty(key, value);
            set(any, info);
        }
    }

    bool isVaild()
    {
        if (kitName().isEmpty() || workspaceFolder().isEmpty() || language().isEmpty())
            return false;
        return true;
    }

    bool isSame(const ProjectInfo &other)
    {
        if (other.kitName() == kitName()
                && other.workspaceFolder() == workspaceFolder()
                && other.language() == language())
            return true;
        return false;
    }

private:
    QVariantHash data;
};

} // namespace dpfservice

Q_DECLARE_METATYPE(dpfservice::ProjectInfo)

#endif // PROJECTINFO_H
