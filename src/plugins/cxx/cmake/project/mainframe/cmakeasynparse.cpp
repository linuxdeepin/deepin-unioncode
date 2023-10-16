// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmakeasynparse.h"
#include "cmakeitemkeeper.h"
#include "cbp/cbpparser.h"
#include "services/project/projectgenerator.h"
#include "services/project/projectservice.h"
#include "properties/targetsmanager.h"
#include "common/common.h"

#include <QAction>
#include <QDebug>

using namespace dpfservice;

namespace {

enum_def(CDT_TARGETS_TYPE, QString)
{
    enum_exp Subprojects = "[Subprojects]";
    enum_exp Targets = "[Targets]";
    enum_exp Lib = "[lib]";
    enum_exp Exe = "[exe]";
};

QIcon cmakeFolderIcon()
{
    static QIcon cmakeFolderIcon;
    if (cmakeFolderIcon.isNull()) {
        cmakeFolderIcon = CustomIcons::icon(QFileIconProvider::Folder);
        cmakeFolderIcon.addFile(":/cmakeproject/images/fileoverlay_cmake@2x.png");
    }
    return cmakeFolderIcon;
}

QIcon libBuildIcon()
{
    static QIcon libBuildIcon;
    if (libBuildIcon.isNull()) {
        libBuildIcon = CustomIcons::icon(CustomIcons::Lib);
        libBuildIcon.addFile(":/cmakeproject/images/build@2x.png");
    }
    return libBuildIcon;
}

QIcon exeBuildIcon()
{
    static QIcon exeBuildIcon;
    if (exeBuildIcon.isNull()) {
        exeBuildIcon = CustomIcons::icon(CustomIcons::Exe);
        // TODO(any):use a different png.
        exeBuildIcon.addFile(":/cmakeproject/images/build@2x.png");
    }
    return exeBuildIcon;
}

}   // namespace

const QString kProjectFile = "CMakeLists.txt";

CmakeAsynParse::CmakeAsynParse()
{
}

CmakeAsynParse::~CmakeAsynParse()
{
}

QString getTargetRootPath(const QList<QString> &srcFiles, const QString &topPath)
{
    if (srcFiles.isEmpty()) {
        return QString();
    }

    // Divide all paths into multiple lists according to directory hierarchy
    QList<QList<QString>> pathPartsList;
    for (const QString &filePath : srcFiles) {
        // remove outter file.
        if ((!topPath.isEmpty() && !filePath.startsWith(topPath)) || QFileInfo(filePath).suffix() == "h" || QFileInfo(filePath).suffix() == "hpp") {
            continue;
        }

        QList<QString> pathParts = filePath.split(QDir::separator());
        pathPartsList.append(pathParts);
    }

    // Find the shortest path list
    int minPathLength = INT_MAX;
    for (const QList<QString> &pathParts : pathPartsList) {
        if (pathParts.length() < minPathLength) {
            minPathLength = pathParts.length();
        }
    }
    if (pathPartsList.size() == 0)
        return {};

    // Starting from the shortest path list, compare whether the paths are the same layer by layer
    QList<QString> rootPathParts;
    for (int i = 0; i < minPathLength; i++) {
        QString currentPart = pathPartsList[0][i];
        bool allMatched = true;
        for (int j = 1; j < pathPartsList.length(); j++) {
            if (pathPartsList[j][i] != currentPart) {
                allMatched = false;
                break;
            }
        }
        if (allMatched) {
            rootPathParts.append(currentPart);
        } else {
            break;
        }
    }

    // Concatenates matching path parts
    QString rootPath = rootPathParts.join(QDir::separator());
    if (rootPath.isEmpty()) {
        rootPath = QDir::separator();
    }

    QFileInfo fileInfo(rootPath);
    if (fileInfo.isFile())
        rootPath = fileInfo.dir().path();

    return rootPath;
}

QStandardItem *CmakeAsynParse::parseProject(QStandardItem *rootItem, const dpfservice::ProjectInfo &prjInfo)
{
    if (!rootItem)
        return nullptr;

    TargetsManager::instance()->readTargets(prjInfo.buildFolder(), prjInfo.workspaceFolder());
    auto cbpParser = TargetsManager::instance()->cbpParser();

    // add cmakefile to tree first.
    auto cmakeList = cbpParser->getCmakeFileList();
    for (auto &cmakeFile : cmakeList) {
        QString cmakeFilePath = cmakeFile.get()->getfilePath();
        QFileInfo cmakeFileInfo(cmakeFilePath);
        if (cmakeFileInfo.fileName().toLower() == kProjectFile.toLower()) {
            auto cmakeParentItem = rootItem;
            QString relativePath = QDir(prjInfo.workspaceFolder()).relativeFilePath(cmakeFileInfo.dir().path());
            if (!relativePath.isEmpty() && relativePath != ".") {
                cmakeParentItem = createParentItem(rootItem, relativePath);
            }
            auto cmakeFileItem = new QStandardItem();
            cmakeFileItem->setIcon(CustomIcons::icon(cmakeFileInfo));
            cmakeFileItem->setText(cmakeFileInfo.fileName());
            cmakeFileItem->setToolTip(cmakeFileInfo.filePath());
            cmakeParentItem->appendRow(cmakeFileItem);

            // monitor cmake file change to refresh project tree.
            if (cmakeParentItem == rootItem) {
                CmakeItemKeeper::instance()->addCmakeRootFile(rootItem, cmakeFilePath);
            } else {
                CmakeItemKeeper::instance()->addCmakeSubFiles(rootItem, {cmakeFilePath});
            }
        }
    }

    QSet<QString> allFiles {};
    const QList<CMakeBuildTarget> &targets = cbpParser->getBuildTargets();
    for (auto target : targets) {
        if (target.type == kUtility) {
            continue;
        }
        QString targetRootPath = getTargetRootPath(target.srcfiles, target.sourceDirectory);
        QString relativePath = QDir(prjInfo.workspaceFolder()).relativeFilePath(QDir(targetRootPath).path());
        QStandardItem *targetRootItem = rootItem;
        if (!relativePath.isEmpty() && relativePath != ".") {
            targetRootItem = createParentItem(rootItem, relativePath);
        }
        QStandardItem *targetItem = new QStandardItem();
        QString prefix = "";
        if (target.type == kExecutable) {
            prefix = CDT_TARGETS_TYPE::get()->Exe;
            targetItem->setIcon(::exeBuildIcon());
        } else if (target.type == kStaticLibrary || target.type == kDynamicLibrary) {
            prefix = CDT_TARGETS_TYPE::get()->Lib;
            targetItem->setIcon(::libBuildIcon());
        }
        QString title = prefix + target.title;
        targetItem->setText(title);

        targetItem->setData(QVariant::fromValue(target));
        targetRootItem->appendRow(targetItem);

        for (const auto &src : target.srcfiles) {
            QFileInfo srcFileInfo(src);
            relativePath = QDir(targetRootPath).relativeFilePath(srcFileInfo.dir().path());
            relativePath.remove(".");
            if (relativePath.startsWith("/"))
                relativePath.remove(0, 1);
            if (srcFileInfo.suffix() == "qm" || srcFileInfo.fileName().startsWith("moc_")
                || srcFileInfo.fileName().startsWith("mocs_")
                || srcFileInfo.fileName().startsWith("qrc_") || srcFileInfo.fileName().startsWith("ui_")) {
                continue;
            }

            QString upDirName = relativePath.split("/").last();
            QStandardItem *parentItem = findItem(targetItem, upDirName, relativePath);
            if (!parentItem) {
                parentItem = createParentItem(targetItem, relativePath);
            }

            QStandardItem *srcItem = new QStandardItem();
            srcItem->setText(srcFileInfo.fileName());
            srcItem->setToolTip(srcFileInfo.filePath());
            srcItem->setIcon(CustomIcons::icon(srcFileInfo));

            parentItem->appendRow(srcItem);

            allFiles.insert(src);
        }
    }

    ProjectInfo tempInfo = prjInfo;
    if (tempInfo.runProgram().isEmpty()) {
        auto activeExecTarget = TargetsManager::instance()->
                getActivedTargetByTargetType(dpfservice::TargetType::kActiveExecTarget);
        tempInfo.setRunProgram(activeExecTarget.output);
        tempInfo.setRunWorkspaceDir(activeExecTarget.workingDir);
    }
    tempInfo.setSourceFiles(allFiles);
    ProjectInfo::set(rootItem, tempInfo);
    emit parseProjectEnd({ rootItem, true });
    return rootItem;
}

QList<CmakeAsynParse::TargetBuild> CmakeAsynParse::parseActions(const QStandardItem *item)
{
    QList<TargetBuild> buildMenuList;

    if (!item)
        return {};

    CMakeBuildTarget buildTarget = item->data().value<CMakeBuildTarget>();
    QStringList commandItems = buildTarget.makeCommand.split(" ");
    commandItems.removeAll("");
    if (commandItems.isEmpty())
        return {};

    TargetBuild build;
    build.buildName = tr("build");
    build.buildCommand = commandItems.first();
    commandItems.pop_front();
    build.buildArguments = commandItems.join(" ");
    build.buildTarget = QFileInfo(buildTarget.output).dir().path();
    buildMenuList.push_back(build);

    parseActionsEnd({ buildMenuList, true });
    return buildMenuList;
}

QStandardItem *CmakeAsynParse::findParentItem(QStandardItem *rootItem, QString &name)
{
    if (!rootItem) {
        return nullptr;
    }
    for (int row = 0; row < rootItem->rowCount(); row++) {
        QStandardItem *childItem = rootItem->child(row);
        QString childDisplayText = childItem->data(Qt::DisplayRole).toString();
        if (name.startsWith(childDisplayText + "/")) {
            QString childName = name;
            childName.remove(0, childDisplayText.size() + 1);
            return findParentItem(childItem, childName);
        }
    }
    return rootItem;
}

QStandardItem *CmakeAsynParse::createParentItem(QStandardItem *rootItem, QString &relativeName)
{
    QStandardItem *retItem = nullptr;
    QStringList nameItems = relativeName.split("/");
    QString preItems;
    for (auto nameItem : nameItems) {
        QString relative = preItems + nameItem;
        QStandardItem *item = findItem(rootItem, nameItem, relative);
        if (!item) {
            // create new one.
            item = new QStandardItem();
            item->setText(nameItem);
            item->setToolTip(relative);
            item->setIcon(::cmakeFolderIcon());
            // append to parent.
            QStandardItem *parentItem = findParentItem(rootItem, relative);
            QString test = parentItem->text();
            parentItem->appendRow(item);
        }
        preItems += nameItem + "/";
        retItem = item;
    }

    return retItem;
}

QStandardItem *CmakeAsynParse::findItem(QStandardItem *rootItem, QString &name, QString &relativePath)
{
    if (!rootItem) {
        return nullptr;
    }

    if (relativePath.isEmpty())
        return rootItem;

    QStandardItem *parentItem = findParentItem(rootItem, relativePath);
    if (parentItem) {
        for (int row = 0; row < parentItem->rowCount(); row++) {
            QStandardItem *childItem = parentItem->child(row);
            if (childItem->data(Qt::DisplayRole) == name) {
                return childItem;
            }
        }
    }
    return nullptr;
}
