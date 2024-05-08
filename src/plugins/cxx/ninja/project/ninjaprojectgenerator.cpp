// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ninjaprojectgenerator.h"
#include "ninjaasynparse.h"

#include "common/dialog/propertiesdialog.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

#include <QtConcurrent>
#include <QtXml>
#include <QFileIconProvider>

class NinjaProjectGeneratorPrivate
{
    friend class NinjaProjectGenerator;
    QStandardItem* configureRootItem {nullptr};
    QMenu *ninjaMenu {nullptr};
    QProcess *menuGenProcess {nullptr};
    QHash<QStandardItem*, NinjaAsynParse*> projectParses {};
};

NinjaProjectGenerator::NinjaProjectGenerator()
    : d(new NinjaProjectGeneratorPrivate())
{
    qRegisterMetaType<QList<QStandardItem*>>("QList<QStandardItem*>");
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService) {
        qCritical() << "Failed, not found service : projectService";
        abort();
    }
}

NinjaProjectGenerator::~NinjaProjectGenerator()
{
    qInfo() << __FUNCTION__;

    if (d)
        delete d;
}

QStringList NinjaProjectGenerator::supportLanguages()
{
    return {dpfservice::MWMFA_CXX};
}

QStringList NinjaProjectGenerator::supportFileNames()
{
    return {"build.ninja"};
}

QDialog *NinjaProjectGenerator::configureWidget(const QString &language,
                                          const QString &projectPath)
{
    using namespace dpfservice;

    ProjectInfo info;
    info.setLanguage(language);
    info.setKitName(NinjaProjectGenerator::toolKitName());
    info.setWorkspaceFolder(projectPath);

    configure(info);

    return nullptr;
}

bool NinjaProjectGenerator::configure(const dpfservice::ProjectInfo &info)
{
    dpfservice::ProjectGenerator::configure(info);

    auto root = createRootItem(info);
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService && root) {
        projectService->addRootItem(root);
        projectService->expandedDepth(root, 1);
    }

    dpfservice::ProjectGenerator::configure(info);

    return true;
}

QStandardItem *NinjaProjectGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;
    QStandardItem * rootItem = ProjectGenerator::createRootItem(info);
    dpfservice::ProjectInfo::set(rootItem, info);
    d->projectParses[rootItem] = new NinjaAsynParse();
    QObject::connect(d->projectParses[rootItem], &NinjaAsynParse::itemsModified,
                     this, &NinjaProjectGenerator::doProjectChildsModified);
    QMetaObject::invokeMethod(d->projectParses[rootItem], "parseProject",
                              Q_ARG(const dpfservice::ProjectInfo &, info));
    return rootItem;
}

void NinjaProjectGenerator::removeRootItem(QStandardItem *root)
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

QMenu *NinjaProjectGenerator::createItemMenu(const QStandardItem *item)
{
    Q_UNUSED(item)
    return nullptr;
}

void NinjaProjectGenerator::doProjectChildsModified(const QList<QStandardItem *> &items)
{
    auto rootItem = d->projectParses.key(qobject_cast<NinjaAsynParse*>(sender()));
    if (rootItem) {
        while (rootItem->hasChildren()) {
            rootItem->takeRow(0);
        }
        rootItem->appendRows(items);
    }
    rootItem->setData(ParsingState::Done, Parsing_State_Role);
}
