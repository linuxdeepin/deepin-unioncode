// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "directorygenerator.h"
#include "directoryasynparse.h"
#include "projectservice.h"

#include <QtConcurrent>

using namespace dpfservice;

struct GeneratorInfo
{
    DirectoryAsynParse *parser { nullptr };
    dpfservice::ProjectInfo projectInfo;
    QStandardItem *item { nullptr };
};

class DirectoryGeneratorPrivate
{
public:
    GeneratorInfo findGeneratorInfo(DirectoryAsynParse *parser);

    QHash<QStandardItem *, GeneratorInfo> projectParses {};
    dpfservice::ProjectInfo prjInfo;
};

GeneratorInfo DirectoryGeneratorPrivate::findGeneratorInfo(DirectoryAsynParse *parser)
{
    auto iter = std::find_if(projectParses.cbegin(), projectParses.cend(),
                             [parser](const GeneratorInfo &info) {
                                 return info.parser == parser;
                             });

    return iter == projectParses.cend() ? GeneratorInfo() : *iter;
}

DirectoryGenerator::DirectoryGenerator()
    : d(new DirectoryGeneratorPrivate())
{
}

DirectoryGenerator::~DirectoryGenerator()
{
    for (auto info : d->projectParses.values())
        if (info.parser)
            delete info.parser;

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
    GeneratorInfo ginfo;
    ginfo.parser = new DirectoryAsynParse();
    ginfo.projectInfo = info;
    ginfo.item = rootItem;
    d->projectParses.insert(rootItem, ginfo);
    QObject::connect(ginfo.parser,
                     &DirectoryAsynParse::itemsCreated,
                     this, &DirectoryGenerator::projectItemsCreated,
                     Qt::UniqueConnection);
    QObject::connect(ginfo.parser,
                     &DirectoryAsynParse::reqUpdateItem,
                     this, &DirectoryGenerator::handleItemUpdated,
                     Qt::UniqueConnection);
    QtConcurrent::run(ginfo.parser, &DirectoryAsynParse::parseProject, info);

    return rootItem;
}

void DirectoryGenerator::removeRootItem(QStandardItem *root)
{
    if (!root)
        return;
    auto info = d->projectParses[root];

    while (root->hasChildren()) {
        root->takeRow(0);
    }
    d->projectParses.remove(root);

    delete root;

    if (info.parser)
        delete info.parser;
}

void DirectoryGenerator::projectItemsCreated(QList<QStandardItem *> itemList)
{
    auto parser = qobject_cast<DirectoryAsynParse *>(sender());
    auto info = d->findGeneratorInfo(parser);
    if (!info.item)
        return;

    auto sourceFiles = parser->getFilelist();
    ProjectInfo tempInfo = info.projectInfo;
    tempInfo.setSourceFiles(sourceFiles);
    ProjectInfo::set(info.item, tempInfo);
    project.activeProject(tempInfo.kitName(), tempInfo.language(), tempInfo.workspaceFolder());

    if (info.item) {
        while (info.item->hasChildren()) {
            info.item->removeRow(0);
        }
        info.item->appendRows(itemList);
    }

    info.item->setData(Project::Done, Project::ParsingStateRole);
}

void DirectoryGenerator::handleItemUpdated(const QString &path)
{
    auto parser = qobject_cast<DirectoryAsynParse *>(sender());
    auto info = d->findGeneratorInfo(parser);
    if (!info.item)
        return;

    ProjectService *prjSrv = dpfGetService(ProjectService);
    Q_ASSERT(prjSrv);

    auto item = parser->findItem(path, info.item);
    if (!item)
        item = info.item;
    parser->updateItem(item);
    if (auto model = info.item->model())
        Q_EMIT model->layoutChanged();
    prjSrv->restoreExpandState(item);
}
