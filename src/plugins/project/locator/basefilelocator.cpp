// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basefilelocator.h"
#include "services/editor/editorservice.h"

#include <DFileIconProvider>

#include <QDebug>

using namespace dpfservice;
using Utils::FileName;

static int matchLevelFor(const QRegularExpressionMatch &match, const QString &matchText)
{
    const int consecutivePos = match.capturedStart(1);
    if (consecutivePos == 0)
        return 0;
    if (consecutivePos > 0) {
        const QChar prevChar = matchText.at(consecutivePos - 1);
        if (prevChar == '_' || prevChar == '.')
            return 1;
    }
    if (match.capturedStart() == 0)
        return 2;
    return 3;
}

baseFileLocator::baseFileLocator(QObject *parent)
    : abstractLocator(parent)
{
    auto &ctx = dpfInstance.serviceContext();
    projectService = ctx.service<ProjectService>(ProjectService::name());
}

void baseFileLocator::prepareSearch(const QString &searchText)
{
    Q_UNUSED(searchText);

    if (fileList.isEmpty())
        return;

    //not to update
    if ((oldFileList == fileList) && !locatorItemList.isEmpty())
        return;

    locatorItemList.clear();
    foreach (QString filePath, fileList) {
        FileName file = FileName::fromUserInput(filePath);

        fileLocatorItem item(this);
        item.filePath = file;
        item.id = file.toString();
        item.tooltip = file.toString();
        auto nativePath = toShortProjectPath(file.toString());
        if (nativePath == file.toString())
            nativePath = file.toShortNativePath();
        item.extraInfo = nativePath;
        QFileInfo fi(filePath);
        item.displayName = fi.fileName();

        auto iconProvider = DTK_WIDGET_NAMESPACE::DFileIconProvider::globalProvider();
        item.icon = iconProvider->icon(fi);

        locatorItemList.append(item);
    }

    std::sort(locatorItemList.begin(), locatorItemList.end(), [=](const fileLocatorItem &itemA, const fileLocatorItem &itemB) {
        return itemA.displayName.toLower() < itemB.displayName.toLower();
    });

    oldFileList = fileList;
}

QList<baseLocatorItem> baseFileLocator::matchesFor(const QString &inputText)
{
    //for priority
    QList<baseLocatorItem> result[4];
    auto regexp = createRegExp(inputText);

    foreach (auto item, locatorItemList) {
        QRegularExpressionMatch match = regexp.match(item.displayName);
        if (match.hasMatch()) {
            auto level = matchLevelFor(match, item.displayName);
            result[level].append(item);
        }
    }

    return result[0] + result[1] + result[2] + result[3];
}

void baseFileLocator::accept(baseLocatorItem item)
{
    editor.openFile(QString(), item.id);
}

void baseFileLocator::setFileList(const QList<QString> &fileList)
{
    this->oldFileList = this->fileList;
    this->fileList = fileList;
}

void baseFileLocator::clear()
{
    locatorItemList.clear();
}

QString baseFileLocator::toShortProjectPath(const QString &path)
{
    QMap<QString, QString> projectList;
    QString nativePath = path;
    if (projectService->getAllProjectInfo) {
        auto allProject = projectService->getAllProjectInfo();
        foreach (auto project, allProject) {
            auto projectRootPath = project.workspaceFolder();
            if (path.startsWith(projectRootPath)) {
                nativePath.replace(0, projectRootPath.size(), QFileInfo(projectRootPath).fileName() + ':');
                break;
            }
        }
    }

    return nativePath;
}
