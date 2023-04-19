#ifndef CMAKEASYNPARSE_H
#define CMAKEASYNPARSE_H

#include "services/project/projectinfo.h"

#include "common/common.h"

#include <QObject>
#include <QtXml>

class QStandardItem;
class QAction;

namespace {
enum_def(CDT_PROJECT_KIT, QString)
{
    enum_exp CDT4_GENERATOR = "Eclipse CDT4 - Unix Makefiles";
    enum_exp CBP_GENERATOR = "CodeBlocks - Unix Makefiles";
    enum_exp PROJECT_FILE = ".project";
    enum_exp CPROJECT_FILE = ".cproject";
    enum_exp CBP_FILE = ".cbp";
};

enum_def(CDT_CPROJECT_KEY, QString)
{
    enum_exp storageModuled = "storageModule";
    enum_exp cconfiguration = "cconfiguration";
    enum_exp buildCommand = "buildCommand";
    enum_exp buildArguments = "buildArguments";
    enum_exp buildTarget = "buildTarget";
    enum_exp stopOnError = "stopOnError";
    enum_exp useDefaultCommand = "useDefaultCommand";
};

} //namespace

class CmakeAsynParse : public QObject
{
    Q_OBJECT
public:
    template<class T>
    struct ParseInfo{
        T result;
        bool isNormal = true;
    };

    struct TargetBuild
    {
        QString buildName;
        QString buildCommand;
        QString buildArguments;
        QString buildTarget;
        QString stopOnError;
        QString useDefaultCommand;
        // 全部都存在数据则有效
        inline bool isInvalid() {
            if (buildName.isEmpty() || buildCommand.isEmpty()
                    || buildArguments.isEmpty() || stopOnError.isEmpty()
                    || useDefaultCommand.isEmpty())
                return true;
            return false;
        }
    };

    typedef QList<TargetBuild> TargetBuilds;

    CmakeAsynParse();
    virtual ~CmakeAsynParse();

signals:
    void parseProjectEnd(const ParseInfo<QStandardItem*> &info);
    void parseActionsEnd(const ParseInfo<QList<TargetBuild>> &info);
    void parseError(const ParseInfo<QString> &info);

public slots:
    QStandardItem *parseProject(QStandardItem *rootItem, const dpfservice::ProjectInfo &info);
    QList<TargetBuild> parseActions(const QStandardItem *item);

private:
    void parseCBP(const QString &buildDir, dpfservice::ProjectInfo &prjInfo);
    QStandardItem *cdt4FindParentItem(QStandardItem *rootItem, QString &name);
    QStandardItem *cdt4FindItem(QStandardItem *rootItem, QString &name);
    QHash<QString, QString> cdt4Subporjects(QStandardItem *rootItem);
    QStandardItem *cdt4DisplayOptimize(QStandardItem *rootItem);
    void cdt4SubprojectsDisplayOptimize(QStandardItem *item);
    void cdt4TargetsDisplayOptimize(QStandardItem *item, const QHash<QString, QString> &subprojectsMap);
    QDomDocument cdt4LoadProjectXmlDoc(const QString &buildFolder);
    QDomDocument cdt4LoadMenuXmlDoc(const QString &buildFolder);
    QDomDocument LoadCBPXmlDoc(const QString &buildFolder);
    QDomDocument loadXmlDoc(const QString &buildFolder, const QString &args);
};

#endif // CMAKEASYNPARSE_H
