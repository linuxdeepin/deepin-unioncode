// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMAKEASYNPARSE_H
#define CMAKEASYNPARSE_H

#include "services/project/projectinfo.h"

#include "common/common.h"

#include <QObject>
#include <QtXml>

class QStandardItem;
class QAction;
class CMakeCbpParser;

namespace {
enum_def(CDT_PROJECT_KIT, QString)
{
    enum_exp CBP_GENERATOR = "CodeBlocks - Unix Makefiles";   
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
}   //namespace

class CmakeAsynParse : public QObject
{
    Q_OBJECT
public:
    template<class T>
    struct ParseInfo
    {
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
    };

    typedef QList<TargetBuild> TargetBuilds;

    CmakeAsynParse();
    virtual ~CmakeAsynParse();

signals:
    void parseProjectEnd(const ParseInfo<QStandardItem *> &info);
    void parseActionsEnd(const ParseInfo<QList<TargetBuild>> &info);

public slots:
    QStandardItem *parseProject(QStandardItem *rootItem, const dpfservice::ProjectInfo &info);
    QList<TargetBuild> parseActions(const QStandardItem *item);

private:
    QStandardItem *findParentItem(QStandardItem *rootItem, QString &name);
    QStandardItem *createParentItem(QStandardItem *rootItem, const QString &relativeName, const QString &absolutePath);
    QStandardItem *findItem(QStandardItem *rootItem, QString &name, QString &relativePath);
};

#endif   // CMAKEASYNPARSE_H
