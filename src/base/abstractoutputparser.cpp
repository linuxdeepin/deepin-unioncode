// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basefilelocator.h"
#include "services/editor/editorservice.h"

#include <DFileIconProvider>
#include <QScopedPointer> 

#include <QDebug>

using namespace dpfservice;
using Utils::FileName;

class BaseFileLocatorPrivate {
public:
    BaseFileLocatorPrivate() = default;
    ProjectService *projectService = nullptr;
    QList<QString> fileList;
    QList<QString> oldFileList; 
    QList<baseLocatorItem> locatorItemList;
};

// aux
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
    : d(new BaseFileLocatorPrivate)
    , abstractLocator(parent) 
{
    auto &ctx = dpfInstance.serviceContext();
    d->projectService = ctx.service<ProjectService>(ProjectService::name());
}

// Destructor
baseFileLocator::~baseFileLocator() 
{
    delete d;
}

void baseFileLocator::prepareSearch(const QString &searchText)
{
    Q_UNUSED(searchText);
    
    if (d->fileList.isEmpty()) {
        qWarning() << "BaseFileLocator: Empty file list.";  // Logging/Debugging
        return;
    }

    if (d->oldFileList == d->fileList && !d->locatorItemList.isEmpty()) {
        return;
    }

    d->locatorItemList.clear();
    for (const QString &filePath : d->fileList) { 
        FileName file = FileName::fromUserInput(filePath);
        baseLocatorItem item;
        item.filePath = file;
        item.id = QObject::tr(file.toString().toUtf8()); // Internacionalizaci¨®n
        item.tooltip = QObject::tr(file.toString().toUtf8()); // Internacionalizaci¨®n
        auto nativePath = toShortProjectPath(file.toString());
        if (nativePath == file.toString()) {
            nativePath = file.toShortNativePath();
        }
        item.extraInfo = nativePath;
        QFileInfo fi(filePath);
        if (!fi.exists()) {
            qWarning() << "BaseFileLocator: File not found:" << filePath; //error handling
            continue; //jump to the next int
        }
        item.displayName = fi.fileName();

        auto iconProvider = DTK_WIDGET_NAMESPACE::DFileIconProvider::globalProvider();
        item.icon = iconProvider->icon(fi);

        d->locatorItemList.append(item);
    }

    // Order by name
    std::sort(d->locatorItemList.begin(), d->locatorItemList.end(), 
              [](const baseLocatorItem &itemA, const baseLocatorItem &itemB) {
                  return itemA.displayName.toLower() < itemB.displayName.toLower();
              });

    d->oldFileList = d->fileList; 
}

QList<baseLocatorItem> baseFileLocator::matchesFor(const QString &inputText) const
{
    QList<baseLocatorItem> result[4];
    auto regexp = createRegExp(inputText);

    for (const auto& item : d->locatorItemList) { 
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
    d->oldFileList = d->fileList;
    d->fileList = fileList;
}

void baseFileLocator::clear()
{
    d->locatorItemList.clear();
}

QString baseFileLocator::toShortProjectPath(const QString &path) const
{
    QMap<QString, QString> projectList;
    QString nativePath = path;
    if (d->projectService->getAllProjectInfo) {
        auto allProject = d->projectService->getAllProjectInfo();
        for (const auto& project : allProject) { // Range-based for loop
            auto projectRootPath = project.workspaceFolder();
            if (path.startsWith(projectRootPath)) {
                nativePath.replace(0, projectRootPath.size(), QFileInfo(projectRootPath).fileName() + ':');
                break;
            }
        }
    }

    return nativePath;
}



