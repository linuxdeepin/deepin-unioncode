// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "llprojectgenerator.h"
#include "common/dialog/propertiesdialog.h"

using namespace dpfservice;

class LLProjectGeneratorPrivate
{
    friend class LLProjectGenerator;
    QHash<QStandardItem *, LLAsynParse *> projectParses {};
};

LLProjectGenerator::LLProjectGenerator()
    : d(new LLProjectGeneratorPrivate)
{
}

LLProjectGenerator::~LLProjectGenerator()
{
    if (d) {
        for (auto parser : d->projectParses.values()) {
            if (parser)
                delete parser;
        }
        delete d;
    }
}

QStringList LLProjectGenerator::supportLanguages()
{
    return { LL_LANGUAGE };
}

QDialog *LLProjectGenerator::configureWidget(const QString &language,
                                             const QString &projectPath)
{
    using namespace dpfservice;

    ProjectInfo info;
    info.setLanguage(language);
    info.setKitName(LLProjectGenerator::toolKitName());
    info.setWorkspaceFolder(projectPath);
    info.setExePrograms({ "LingLong" });

    configure(info);

    return nullptr;
}

bool LLProjectGenerator::configure(const dpfservice::ProjectInfo &projectInfo)
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

QStandardItem *LLProjectGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;

    QStandardItem *rootItem = ProjectGenerator::createRootItem(info);
    d->projectParses[rootItem] = new LLAsynParse();
    QObject::connect(d->projectParses[rootItem],
                     &LLAsynParse::itemsModified,
                     this, &LLProjectGenerator::doProjectChildsModified,
                     Qt::ConnectionType::UniqueConnection);
    d->projectParses[rootItem]->parseProject(info);
    auto sourceFiles = d->projectParses[rootItem]->getFilelist();
    dpfservice::ProjectInfo tempInfo = info;
    tempInfo.setSourceFiles(sourceFiles);
    dpfservice::ProjectInfo::set(rootItem, tempInfo);

    return rootItem;
}

void LLProjectGenerator::removeRootItem(QStandardItem *root)
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

QMenu *LLProjectGenerator::createItemMenu(const QStandardItem *item)
{
    if (item->parent())
        return nullptr;

    QMenu *menu = new QMenu();
    //    dpfservice::ProjectInfo info = dpfservice::ProjectInfo::get(item);
    //    if (info.isEmpty())
    //        return nullptr;

    //    QStandardItem *itemTemp = const_cast<QStandardItem *>(item);
    //    if (!itemTemp)
    //        return nullptr;

    //    QAction *action = new QAction(tr("Properties"));
    //    menu->addAction(action);
    //    QObject::connect(action, &QAction::triggered, [=]() {
    //        actionProperties(info, itemTemp);
    //    });

    return menu;
}

void LLProjectGenerator::doProjectChildsModified(const QList<QStandardItem *> &info)
{
    auto rootItem = d->projectParses.key(qobject_cast<LLAsynParse *>(sender()));
    if (rootItem) {
        while (rootItem->hasChildren()) {
            rootItem->takeRow(0);
        }
        rootItem->appendRows(info);
    }

    rootItem->setData(ParsingState::Done, Parsing_State_Role);
}

void LLProjectGenerator::actionProperties(const ProjectInfo &info, QStandardItem *item)
{
    //PropertiesDialog dlg;
    //    ConfigPropertyWidget *property = new ConfigPropertyWidget(info, item);
    //    dlg.insertPropertyPanel("Config", property);
    //    dlg.exec();
}
