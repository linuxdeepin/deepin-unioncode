// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "directorygenerator.h"
#include "directoryasynparse.h"
#include "projectservice.h"

#include <QtConcurrent>

using namespace dpfservice;
class DirectoryGeneratorPrivate
{
    friend class dpfservice::DirectoryGenerator;
    QStandardItem *configureRootItem { nullptr };
    QHash<QStandardItem *, DirectoryAsynParse *> projectParses {};
    QMap<QStandardItem *, dpfservice::ProjectInfo> allProjectInfo;
    dpfservice::ProjectInfo prjInfo;
};

DirectoryGenerator::DirectoryGenerator()
    : d(new DirectoryGeneratorPrivate())
{
}

DirectoryGenerator::~DirectoryGenerator()
{
    for (auto parser : d->projectParses.values())
        if(parser)
            delete parser;

    if (d)
        delete d;
}

DWidget *DirectoryGenerator::configureWidget(const QString &language,
                                             const QString &projectPath)
{
    using namespace dpfservice;

    prjInfo.setLanguage(language);
    prjInfo.setKitName(configureKitName());
    prjInfo.setWorkspaceFolder(projectPath);

    return nullptr;
}

void DirectoryGenerator::acceptConfigure()
{
    configure(prjInfo);
}

bool DirectoryGenerator::configure(const dpfservice::ProjectInfo &projectInfo)
{
    dpfservice::ProjectGenerator::configure(projectInfo);

    auto root = createRootItem(projectInfo);
    d->allProjectInfo.insert(root, projectInfo);
    ProjectInfo::set(root, projectInfo);
    ProjectService *projectService = dpfGetService(ProjectService);
    if (projectService && root) {
        projectService->addRootItem(root);
        projectService->expandedDepth(root, 1);
    }

    return true;
}

QStandardItem *DirectoryGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;

    QStandardItem *rootItem = ProjectGenerator::createRootItem(info);
    d->configureRootItem = rootItem;
    auto parser = new DirectoryAsynParse();
    d->projectParses[rootItem] = parser;
    QObject::connect(d->projectParses[rootItem],
                     &DirectoryAsynParse::itemsModified,
                     this, &DirectoryGenerator::doProjectChildsModified,
                     Qt::UniqueConnection);
    QObject::connect(d->projectParses[rootItem],
                     &DirectoryAsynParse::parseDone,
                     this, [&info](){
                         project.activeProject(info.kitName(), info.language(), info.workspaceFolder());
                     },
                     Qt::UniqueConnection);
    QtConcurrent::run(parser, &DirectoryAsynParse::parseProject, info);

    return rootItem;
}

void DirectoryGenerator::removeRootItem(QStandardItem *root)
{
    if (!root)
        return;
    auto parser = d->projectParses[root];

    while (root->hasChildren()) {
        root->takeRow(0);
    }
    d->projectParses.remove(root);

    delete root;

    if (parser)
        delete parser;
}

void DirectoryGenerator::doProjectChildsModified(const QList<QStandardItem *> &info)
{
    auto modifiedItem = d->configureRootItem;
    if (sender() && (d->projectParses[d->configureRootItem] != sender())) {
        auto parser = qobject_cast<DirectoryAsynParse *>(sender());
        modifiedItem = d->projectParses.key(parser);
    }
    auto sourceFiles = d->projectParses[modifiedItem]->getFilelist();
    auto modifiedProjectInfo = ProjectInfo::get(modifiedItem);
    ProjectInfo tempInfo = modifiedProjectInfo;
    tempInfo.setSourceFiles(sourceFiles);
    ProjectInfo::set(modifiedItem, tempInfo);

    auto rootItem = d->projectParses.key(qobject_cast<DirectoryAsynParse *>(sender()));
    if (rootItem) {
        while (rootItem->hasChildren()) {
            rootItem->takeRow(0);
        }
        rootItem->appendRows(info);
    }

    rootItem->setData(ParsingState::Done, ProjectItemRole::ParsingStateRole);
}
