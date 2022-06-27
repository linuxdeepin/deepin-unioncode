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

class GradleAsynParsePrivate;
class GradleAsynParse : public QObject
{
    Q_OBJECT
    GradleAsynParsePrivate  *const d;
public:
    template<class T>
    struct ParseInfo{
        T result;
        bool isNormal = true;
    };

    GradleAsynParse();
    virtual ~GradleAsynParse();

signals:
    void parsedProject(const ParseInfo<QList<QStandardItem*>> &info);
    void parsedError(const ParseInfo<QString> &info);

public slots:
    void loadPoms(const dpfservice::ProjectInfo &info);
    void parseProject(const dpfservice::ProjectInfo &info);

private slots:
    void doDirWatchModify(const QString &path);
    void doWatchCreatedSub(const QString &path);
    void doWatchDeletedSub(const QString &path);

private:
    bool isSame(QStandardItem *t1, QStandardItem *t2, Qt::ItemDataRole role = Qt::DisplayRole) const;
    QList<QStandardItem *> generatorChildItem(const QString &path) const;
    QList<QStandardItem *> parents(QStandardItem *item) const;
    QString path(QStandardItem *item) const;
    QList<QStandardItem*> rows(const QStandardItem *item) const;
    int findRowWithDisplay(QStandardItem *item, const QString &fileName);
    int findRowWithDisplay(QList<QStandardItem*> rowList, const QString &fileName);
    QStandardItem *findItem(QList<QStandardItem*> rowList, QString &path, QStandardItem *parent = nullptr) const;
};

#endif // CMAKEASYNPARSE_H
