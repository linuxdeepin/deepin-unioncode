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
    auto parser = new DirectoryAsynParse();
    d->projectParses[rootItem] = parser;
    QObject::connect(d->projectParses[rootItem],
                     &DirectoryAsynParse::itemsCreated,
                     this, &DirectoryGenerator::projectItemsCreated,
                     Qt::UniqueConnection);
    QObject::connect(d->projectParses[rootItem],
                     &DirectoryAsynParse::reqUpdateItem,
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

void DirectoryGenerator::projectItemsCreated(QList<QStandardItem *> itemList)
{
    auto sourceFiles = d->projectParses[d->configureRootItem]->getFilelist();
    ProjectInfo tempInfo = prjInfo;
    tempInfo.setSourceFiles(sourceFiles);
    ProjectInfo::set(d->configureRootItem, tempInfo);
    project.activeProject(tempInfo.kitName(), tempInfo.language(), tempInfo.workspaceFolder());

    auto rootItem = d->projectParses.key(qobject_cast<DirectoryAsynParse *>(sender()));
    if (rootItem) {
        while (rootItem->hasChildren()) {
            rootItem->removeRow(0);
        }
        rootItem->appendRows(itemList);
    }

    rootItem->setData(Project::Done, Project::ParsingStateRole);
}

void DirectoryGenerator::handleItemUpdated(const QString &path)
{
    ProjectService *prjSrv = dpfGetService(ProjectService);
    Q_ASSERT(prjSrv);

    auto parser = qobject_cast<DirectoryAsynParse *>(sender());
    auto rootItem = d->projectParses.key(parser);
    auto item = parser->findItem(path, rootItem);
    if (!item)
        item = rootItem;
    parser->updateItem(item);
    if (auto model = rootItem->model())
        Q_EMIT model->layoutChanged();
    prjSrv->restoreExpandState(item);
}
