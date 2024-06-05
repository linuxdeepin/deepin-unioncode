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

#include <DIconTheme>

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
    }
    return cmakeFolderIcon;
}

}   // namespace

const QString kProjectFile = "CMakeLists.txt";

void sortParentItem(QStandardItem *parentItem)
{
    QList<QStandardItem *> cmakeFileList;
    QList<QStandardItem *> fileList;
    QList<QStandardItem *> directoryList;
    QList<QStandardItem *> others;

    int count = parentItem->rowCount();

    for (int row = 0; row < count; row++) {
        auto child = parentItem->child(row);
        if (child->text() == kProjectFile) {
            cmakeFileList.append(parentItem->takeChild(row));
            continue;
        }

        auto absolutePath = child->toolTip();
        QFileInfo fileInfo(absolutePath);
        if (fileInfo.isFile())
            fileList.append(parentItem->takeChild(row));
        else if (fileInfo.isDir())
            directoryList.append(parentItem->takeChild(row));
        else
            others.append(parentItem->takeChild(row));
    }
    parentItem->removeRows(0, parentItem->rowCount());

    //fileList is already sorted
    std::sort(directoryList.begin(), directoryList.end(), [](const QStandardItem *item1, const QStandardItem *item2) {
        return item1->text().toLower().localeAwareCompare(item2->text().toLower()) < 0;
    });

    for ( auto item : cmakeFileList )
        parentItem->appendRow(item);
    for ( auto item : directoryList )
        parentItem->appendRow(item);
    for ( auto item : fileList )
        parentItem->appendRow(item);
    for ( auto item : others )
        parentItem->appendRow(item);

}

CmakeAsynParse::CmakeAsynParse()
{
}

CmakeAsynParse::~CmakeAsynParse()
{
}

QString getTargetRootPath(const CMakeBuildTarget &target, const dpfservice::ProjectInfo &prjInfo)
{
    auto srcFiles = target.srcfiles;
    auto topPath = target.sourceDirectory;

    if (srcFiles.isEmpty()) {
        return QString();
    }

    //get target root path by build-directory
    auto workingDirectory = target.workingDirectory;
    auto buildDirectory = prjInfo.buildFolder();
    if(workingDirectory.startsWith(buildDirectory)) {
        workingDirectory.remove(buildDirectory);
        return topPath + QDir::separator() + workingDirectory;
    }

    //get target root path by srcFiles path
    // Divide all paths into multiple lists according to directory hierarchy
    QList<QList<QString>> pathPartsList;
    for (const QString &filePath : srcFiles) {
        // remove outter file.
        if ((!topPath.isEmpty() && !filePath.startsWith(topPath))/* || QFileInfo(filePath).suffix() == "h" || QFileInfo(filePath).suffix() == "hpp"*/) {
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
    QSet<QString> cmakeFiles {};
    for (auto &cmakeFile : cmakeList) {
        QString cmakeFilePath = cmakeFile.get()->getfilePath();
        if (cmakeFilePath.endsWith("CMakeLists.txt"))
            cmakeFiles.insert(cmakeFilePath);
        QFileInfo cmakeFileInfo(cmakeFilePath);
        if (cmakeFileInfo.fileName().toLower() == kProjectFile.toLower()) {
            auto cmakeParentItem = rootItem;
            QString relativePath = QDir(prjInfo.workspaceFolder()).relativeFilePath(cmakeFileInfo.dir().path());
            QString absolutePath = QDir(prjInfo.workspaceFolder()).absoluteFilePath(cmakeFileInfo.dir().path());
            if (!relativePath.isEmpty() && relativePath != ".") {
                cmakeParentItem = createParentItem(rootItem, relativePath, absolutePath);
            }

            auto cmakeFileItem = new QStandardItem();
            cmakeFileItem->setText(cmakeFileInfo.fileName());
            cmakeFileItem->setToolTip(cmakeFileInfo.filePath());
            cmakeParentItem->appendRow(cmakeFileItem);
            QMetaObject::invokeMethod(this, [=](){
                cmakeFileItem->setIcon(CustomIcons::icon(cmakeFileInfo));
            });

            // monitor cmake file change to refresh project tree.
            if (cmakeParentItem == rootItem) {
                CmakeItemKeeper::instance()->addCmakeRootFile(rootItem, cmakeFilePath);
            } else {
                CmakeItemKeeper::instance()->addCmakeSubFiles(rootItem, {cmakeFilePath});
            }
        }
    }

    QSet<QString> commonFiles {};
    const QList<CMakeBuildTarget> &targets = cbpParser->getBuildTargets();
    for (auto target : targets) {
        if (target.type == kUtility) {
            continue;
        }
        QString targetRootPath = getTargetRootPath(target, prjInfo);
        if (!QFileInfo(targetRootPath).exists())
            continue;
        QString relativePath = QDir(prjInfo.workspaceFolder()).relativeFilePath(QDir(targetRootPath).path());
        QString absolutePath = QDir(prjInfo.workspaceFolder()).absoluteFilePath(QDir(targetRootPath).path());
        QStandardItem *targetRootItem = rootItem;
        if (!relativePath.isEmpty() && relativePath != ".") {
            targetRootItem = createParentItem(rootItem, relativePath, absolutePath);
        }
        QStandardItem *targetItem = new QStandardItem();
        QString prefix = "";
        if (target.type == kExecutable) {
            prefix = CDT_TARGETS_TYPE::get()->Exe;
        } else if (target.type == kStaticLibrary || target.type == kDynamicLibrary) {
            prefix = CDT_TARGETS_TYPE::get()->Lib;
        }
        QMetaObject::invokeMethod(this, [=](){
            // executable should different with library.
            targetItem->setIcon(DIconTheme::findQIcon("project_executable"));
        });
        QString title = prefix + target.title;
        targetItem->setText(title);
        targetItem->setToolTip(absolutePath);

        targetItem->setData(QVariant::fromValue(target));
        if (targetRootItem)
            targetRootItem->appendRow(targetItem);

        for (const auto &src : target.srcfiles) {
            QFileInfo srcFileInfo(src);
            relativePath = QDir(targetRootPath).relativeFilePath(srcFileInfo.dir().path());
            absolutePath = QDir(targetRootPath).absoluteFilePath(srcFileInfo.dir().path());
            relativePath.remove(".");
            while (relativePath.startsWith("/"))
                relativePath.remove(0, 1);
            if (srcFileInfo.suffix() == "qm" || srcFileInfo.fileName().startsWith("moc_")
                || srcFileInfo.fileName().startsWith("mocs_")
                || srcFileInfo.fileName().startsWith("qrc_") || srcFileInfo.fileName().startsWith("ui_")) {
                continue;
            }

            QString upDirName = relativePath.split("/").last();
            QStandardItem *parentItem = findItem(targetItem, upDirName, relativePath);
            if (!parentItem) {
                parentItem = createParentItem(targetItem, relativePath, absolutePath);
            }

            QStandardItem *srcItem = new QStandardItem();
            srcItem->setText(srcFileInfo.fileName());
            srcItem->setToolTip(srcFileInfo.filePath());
            srcItem->setIcon(CustomIcons::icon(srcFileInfo));

            if (parentItem)
                parentItem->appendRow(srcItem);

            commonFiles.insert(src);
        }
    }

    ProjectInfo tempInfo = prjInfo;
    if (tempInfo.runProgram().isEmpty()) {
        auto activeExecTarget = TargetsManager::instance()->
                getActivedTargetByTargetType(dpfservice::TargetType::kActiveExecTarget);
        tempInfo.setRunProgram(activeExecTarget.output);
        tempInfo.setRunWorkspaceDir(activeExecTarget.workingDir);
    }

    tempInfo.setSourceFiles(commonFiles + cmakeFiles);
    tempInfo.setExePrograms(TargetsManager::instance()->getExeTargetNamesList());
    tempInfo.setCurrentProgram(TargetsManager::instance()->getActivedTargetByTargetType(TargetType::kActiveExecTarget).name);
    ProjectInfo::set(rootItem, tempInfo);
    emit parseProjectEnd({ rootItem, true });
    rootItem->setData(ParsingState::Done, Parsing_State_Role);

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

QStandardItem *CmakeAsynParse::createParentItem(QStandardItem *rootItem, const QString &relativeName, const QString &absolutePath)
{
    QStandardItem *retItem = nullptr;
    QString basePath = absolutePath.mid(0, absolutePath.length() - relativeName.length());
    QStringList nameItems = relativeName.split("/");
    QString preItems;
    for (auto nameItem : nameItems) {
        QString relative = preItems + nameItem;
        QStandardItem *item = findItem(rootItem, nameItem, relative);
        if (!item) {
            // create new one.
            item = new QStandardItem();
            item->setText(nameItem);
            item->setToolTip(basePath + relative);
            item->setIcon(::cmakeFolderIcon());
            // append to parent.
            QStandardItem *parentItem = findParentItem(rootItem, relative);
            parentItem->appendRow(item);
            sortParentItem(parentItem);
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
