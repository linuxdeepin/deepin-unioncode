// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PROJECTINFO_H
#define PROJECTINFO_H

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

    inline void setLanguage(const QString &language) {data["Language"] = language;}
    inline QString language() const {return data["Language"].toString();}
    inline void setKitName(const QString &kitName) {data["KitName"] = kitName;}
    inline QString kitName() const {return data["KitName"].toString();}
    inline void setBuildFolder(const QString &buildPath) {data["BuildFolder"] = buildPath;}
    inline QString buildFolder() const {return data["BuildFolder"].toString();}
    inline void setWorkspaceFolder(const QString &workspaceFolder) {data["WorkspaceFolder"] = workspaceFolder;}
    inline QString workspaceFolder() const {return data["WorkspaceFolder"].toString();}
    inline void setBuildType(const QString &buildType) {data["BuildType"] = buildType;}
    inline QString buildType() const {return data["BuildType"].toString();}
    inline void setBuildProgram(const QString &program) {data["BuildProgram"] = program;}
    inline QString buildProgram() const {return data["BuildProgram"].toString();}
    inline void setConfigCustomArgs(const QStringList &args) {data["ConfigCustomArgs"] = args;}
    inline QStringList configCustomArgs() const {return data["ConfigCustomArgs"].toStringList();}
    inline void setSourceFiles(const QSet<QString> &files) {data["SourceFiles"] = QVariant::fromValue(files);}
    inline QSet<QString> sourceFiles() const {return qvariant_cast<QSet<QString>>(data["SourceFiles"]);}
    inline void setDetailInformation(bool isDetail) {data["DetailInformation"] = isDetail;}
    inline bool detailInformation() const {return data["DetailInformation"].toBool();}
    inline void setBuildCustomArgs(const QStringList &args) {data["BuildCustomArgs"] = args;}
    inline QStringList buildCustomArgs() const {return data["BuildCustomArgs"].toStringList();}
    inline void setCleanCustomArgs(const QStringList &args) {data["CleanCustomArgs"] = args;}
    inline QStringList cleanCustomArgs() const {return data["CleanCustomArgs"].toStringList();}
    inline void setRunProgram(const QString &program) {data["RunProgram"] = program;}
    inline QString runProgram() const {return data["RunProgram"].toString();}
    inline void setExePrograms(const QStringList &args) {data["exePrograms"] = args;}
    inline QStringList exePrograms() const {return data["exePrograms"].toStringList();}
    inline void setCurrentProgram(const QString &arg) {data["currentProgram"] = arg;}
    inline QString currentProgram() const {return data["currentProgram"].toString();}
    inline void setRunCustomArgs(const QStringList &args) {data["RunCustomArgs"] = args;}
    inline QStringList runCustomArgs() const {return data["RunCustomArgs"].toStringList();}
    inline void setRunWorkspaceDir(const QString &workspaceDir) {data["RunWorkspaceDir"] = workspaceDir;}
    inline QString runWorkspaceDir() const {return data["RunWorkspaceDir"].toString();}
    inline void setRunEnvironment(const QStringList &envs) {data["RunEnvironment"] = envs;}
    inline QStringList runEnvironment() const {return data["RunEnvironment"].toStringList();}

    bool operator == (const ProjectInfo &other) const {return data == other.data;}
    bool isEmpty() const {return data.isEmpty();}
    inline bool hasKey(const QString &key) {return data.keys().contains(key);}
    inline void setProperty(const QString &key, QVariant value) {data[key] = value;}
    inline QVariant property(const QString &key) const {return data[key];}

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

    friend QDataStream &operator<<(QDataStream &stream, ProjectInfo &projInfo)
    {
        stream << projInfo.data;
        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, ProjectInfo &projInfo)
    {
        stream >> projInfo.data;
        return stream;
    }

private:
    QVariantHash data;
};

} // namespace dpfservice

Q_DECLARE_METATYPE(dpfservice::ProjectInfo)

#endif // PROJECTINFO_H
