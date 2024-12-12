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
    dpfservice::ProjectInfo prjInfo;
};

DirectoryGenerator::DirectoryGenerator()
    : d(new DirectoryGeneratorPrivate())
{
}

DirectoryGenerator::~DirectoryGenerator()
{
    for (auto parser : d->projectParses.values())
        if (parser)
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
    auto parser = new DirectoryAsynParse(rootItem);
    d->projectParses[rootItem] = parser;
    QObject::connect(d->projectParses[rootItem],
                     &DirectoryAsynParse::itemsModified,
                     this, &DirectoryGenerator::doProjectChildsModified,
                     Qt::UniqueConnection);
    QObject::connect(d->projectParses[rootItem],
                     &DirectoryAsynParse::itemUpdated,
                     this, &DirectoryGenerator::handleItemUpdated,
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

void DirectoryGenerator::doProjectChildsModified()
{
    auto sourceFiles = d->projectParses[d->configureRootItem]->getFilelist();
    ProjectInfo tempInfo = prjInfo;
    tempInfo.setSourceFiles(sourceFiles);
    ProjectInfo::set(d->configureRootItem, tempInfo);
    project.activeProject(tempInfo.kitName(), tempInfo.language(), tempInfo.workspaceFolder());

    auto rootItem = d->projectParses.key(qobject_cast<DirectoryAsynParse *>(sender()));
    rootItem->setData(Project::Done, Project::ParsingStateRole);
}

void DirectoryGenerator::handleItemUpdated(QStandardItem *item)
{
    ProjectService *prjSrv = dpfGetService(ProjectService);
    Q_ASSERT(prjSrv);

    prjSrv->restoreExpandState(item);
}
