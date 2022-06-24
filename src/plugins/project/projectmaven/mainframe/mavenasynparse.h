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
    enum_exp PROJECT_FILE = ".project";
    enum_exp CPROJECT_FILE = ".cproject";
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

class MavenAsynParsePrivate;
class MavenAsynParse : public QObject
{
    Q_OBJECT
    MavenAsynParsePrivate  *const d;
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

    MavenAsynParse();
    virtual ~MavenAsynParse();

signals:
    void parsedProject(const ParseInfo<QList<QStandardItem*>> &info);
    void parsedActions(const ParseInfo<QList<TargetBuild>> &info);
    void parsedError(const ParseInfo<QString> &info);

public slots:
    void loadPoms(const dpfservice::ProjectInfo &info);
    void parseProject(const dpfservice::ProjectInfo &info);
    void parseActions(const QStandardItem *item);
private:
    bool isSame(QStandardItem *t1, QStandardItem *t2, Qt::ItemDataRole role = Qt::DisplayRole) const;
    QList<QStandardItem *> generatorChildItem(const QString &path) const;
    QList<QStandardItem *> parents(QStandardItem *item) const;
    QString path(QStandardItem *item) const;
    QList<QStandardItem*> rows(const QStandardItem *item) const;
    QStandardItem *findItem(QList<QStandardItem*> rowList, QString &path, QStandardItem *parent = nullptr) const;
    QList<QStandardItem*> createFileItem(QList<QStandardItem*> item, const QString &Path) const;
};

#endif // CMAKEASYNPARSE_H
