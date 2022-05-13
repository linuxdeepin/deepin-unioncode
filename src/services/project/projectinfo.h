#ifndef PROJECTINFO_H
#define PROJECTINFO_H

#include <QVariantHash>
#include <QStandardItem>

namespace dpfservice {

enum ItemDataRole
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
    inline void setSourceFolder(const QString &sourcePath) {data["SourceFolder"] = sourcePath;}
    inline void setWorkspaceFolder(const QString &workspaceFolder) {data["WorkspaceFolder"] = workspaceFolder;}
    inline void setProjectFilePath(const QString &projectFilePath) {data["ProjectFilePath"] = projectFilePath;}
    inline void setBuildType(const QString &buildType) {data["BuildType"] = buildType;}
    inline void setBuildCustomArgs(const QStringList &args) {data["BuildCustomArgs"] = args;}

    inline QString language() const {return data["Language"].toString();}
    inline QString kitName() const {return data["KitName"].toString();}
    inline QString buildFolder() const {return data["BuildFolder"].toString();}
    inline QString sourceFolder() const {return data["SourceFolder"].toString();}
    inline QString workspaceFolder() const {return data["WorkspaceFolder"].toString();}
    inline QString projectFilePath() const {return data["ProjectFilePath"].toString();}
    inline QString buildType() const {return data["BuildType"].toString();}
    inline QStringList buildCustomArgs() const {return data["BuildCustomArgs"].toStringList();}

    inline static void set(QStandardItem *root, const ProjectInfo &info)
    {
        if (root)
            root->setData(QVariant::fromValue(info), ItemDataRole::ProjectInfoRole);
    }

    inline static ProjectInfo get(const QModelIndex &root)
    {
        if (root.isValid())
            return qvariant_cast<ProjectInfo>(root.data(ItemDataRole::ProjectInfoRole));
        return {};
    }

    inline static ProjectInfo get(const QStandardItem *root)
    {
        if (root)
            return qvariant_cast<ProjectInfo>(root->data(ItemDataRole::ProjectInfoRole));
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

private:
    QVariantHash data;
};

} // namespace dpfservice

Q_DECLARE_METATYPE(dpfservice::ProjectInfo)

#endif // PROJECTINFO_H
