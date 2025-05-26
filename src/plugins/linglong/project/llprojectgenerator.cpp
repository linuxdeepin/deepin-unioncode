// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "llprojectgenerator.h"
#include "common/dialog/propertiesdialog.h"

using namespace dpfservice;

LLProjectGenerator::LLProjectGenerator()
{
}

LLProjectGenerator::~LLProjectGenerator()
{
}

QStringList LLProjectGenerator::supportLanguages()
{
    return { LL_LANGUAGE };
}

DWidget *LLProjectGenerator::configureWidget(const QString &language,
                                             const QString &projectPath)
{
    using namespace dpfservice;

    prjInfo.setLanguage(language);
    prjInfo.setKitName(LLProjectGenerator::toolKitName());
    prjInfo.setWorkspaceFolder(projectPath);
    prjInfo.setExePrograms({ "LingLong" });

    return nullptr;
}

void LLProjectGenerator::acceptConfigure()
{
    configure(prjInfo);
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

void LLProjectGenerator::actionProperties(const ProjectInfo &info, QStandardItem *item)
{

}
