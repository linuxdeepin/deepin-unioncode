// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QRegularExpression>

ProjectNode *Utils::createProjectNode(const dpfservice::ProjectInfo &info)
{
    if (info.isEmpty())
        return nullptr;

    ProjectNode *prjNode = new ProjectNode(info.workspaceFolder());
    std::vector<std::unique_ptr<FileNode>> sourceNodes;
    const auto &sources = info.sourceFiles();
    std::transform(sources.cbegin(), sources.cend(), std::back_inserter(sourceNodes),
                   [](const QString &f) {
                       return std::make_unique<FileNode>(f);
                   });
    if (!sourceNodes.empty())
        prjNode->addNestedNodes(std::move(sourceNodes), info.workspaceFolder());

    return prjNode;
}

FolderNode *Utils::recursiveFindOrCreateFolderNode(FolderNode *folder,
                                                   const QString &directory,
                                                   const QString &workspace,
                                                   const FolderNode::FolderNodeFactory &factory)
{
    QString path = workspace.isEmpty() ? folder->filePath() : workspace;
    QString directoryWithoutPrefix;
    bool isRelative = false;

    if (path.isEmpty() || path == "/") {
        directoryWithoutPrefix = directory;
        isRelative = false;
    } else {
        if (isChildOf(path, directory) || directory == path) {
            isRelative = true;
            directoryWithoutPrefix = relativeChildPath(path, directory);
        } else {
            const QString relativePath = relativeChildPath(path, directory);
            if (relativePath.count("../") < 5) {
                isRelative = true;
                directoryWithoutPrefix = relativePath;
            } else {
                isRelative = false;
                path.clear();
                directoryWithoutPrefix = directory;
            }
        }
    }
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList parts = directoryWithoutPrefix.split('/', Qt::SkipEmptyParts);
#else
    QStringList parts = directoryWithoutPrefix.split('/', QString ::SkipEmptyParts);
#endif
    if (!isRelative && !parts.isEmpty())
        parts[0].prepend('/');

    FolderNode *parent = folder;
    for (const QString &part : std::as_const(parts)) {
        path += QLatin1Char('/') + part;
        // Find folder in subFolders
        FolderNode *next = parent->folderNode(path);
        if (!next) {
            // No FolderNode yet, so create it
            auto tmp = factory(path);
            tmp->setDisplayName(part);
            next = tmp.get();
            parent->addNode(std::move(tmp));
        }
        parent = next;
    }
    return parent;
}

bool Utils::isChildOf(const QString &path, const QString &subPath)
{
    if (path.isEmpty() || subPath.isEmpty())
        return false;

    if (!subPath.startsWith(path))
        return false;
    if (subPath.size() <= path.size())
        return false;
    if (path.endsWith(QLatin1Char('/')))
        return true;
    return subPath.at(path.size()) == QLatin1Char('/');
}

QString Utils::relativeChildPath(const QString &path, const QString &subPath)
{
    QString res;
    if (isChildOf(path, subPath)) {
        res = subPath.mid(path.size());
        if (res.startsWith('/'))
            res = res.mid(1);
    }
    return res;
}

bool Utils::isValidPath(const QString &path)
{
    static QRegularExpression regex(R"(^(/[^/ ]*)+/?$)");
    return regex.match(path).hasMatch();
}

bool Utils::isCppFile(const QString &filePath)
{
    static QStringList extensions = { "h", "hpp", "c", "cpp", "cc" };

    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    return extensions.contains(suffix);
}

bool Utils::isCMakeFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.fileName().compare("CMakeLists.txt", Qt::CaseInsensitive) == 0;
}

QStringList Utils::relateFileList(const QString &filePath)
{
    QStringList relatedFileList;
    if (isCppFile(filePath)) {
        QFileInfo fileInfo(filePath);
        QDir dir(fileInfo.absolutePath());
        QStringList filters { "*.h", "*.hpp", "*.c", "*.cpp", "*.cc" };
        QStringList allFiles = dir.entryList(filters, QDir::Files);

        bool containsHeader = false;
        QString baseName = fileInfo.baseName();
        for (const auto &file : allFiles) {
            QFileInfo info(file);
            if (info.baseName() == baseName) {
                relatedFileList << dir.absoluteFilePath(file);
                if (!containsHeader)
                    containsHeader = (info.suffix() == "h" || info.suffix() == "hpp");
            }
        }

        if (!containsHeader)
            return {};
    } else {
        relatedFileList << filePath;
    }

    return relatedFileList;
}

QString Utils::createUTFile(const QString &workspace, const QString &filePath,
                            const QString &target, const QString &nameFormat)
{
    QFileInfo info(filePath);
    QString utFile;
    if (isCppFile(filePath)) {
        utFile = info.absolutePath().replace(workspace, target);
        QString targetName = info.baseName();
        QString format = nameFormat;
        format.replace("${filename}", targetName);
        utFile += QDir::separator() + format;
    } else {
        utFile = info.absoluteFilePath().replace(workspace, target);
    }

    return utFile;
}

QString Utils::createRequestPrompt(const FileNode *node, const QString &chunkPrompt, const QString &userPrompt)
{
    QStringList prompt;
    prompt << "##### Prompt #####"
           << "<system>"
           << "You are an expert software developer. You give helpful and concise responses.\n"
           << "<user>";

    QStringList fileContents;
    for (const auto &file : node->sourceFiles()) {
        fileContents << "```" + file;
        QFile f(file);
        if (f.open(QIODevice::ReadOnly))
            fileContents << f.readAll();
        fileContents << "```\n";
    }

    if (!chunkPrompt.isEmpty())
        prompt << chunkPrompt;
    prompt << fileContents;
    prompt << node->sourceFiles().join(" ");
    prompt << userPrompt;

    return prompt.join('\n');
}

QString Utils::createChunkPrompt(const QJsonObject &chunkObj)
{
    QJsonArray chunks = chunkObj["Chunks"].toArray();
    if (chunks.isEmpty())
        return {};

    QStringList chunkPrompt("Use the above code to answer the following question. "
                            "You should not reference any files outside of what is shown, "
                            "unless they are commonly known files, like a .gitignore or "
                            "package.json. Reference the filenames whenever possible. If "
                            "there isn't enough information to answer the question, suggest "
                            "where the user might look to learn more.\n");
    for (auto chunk : chunks) {
        chunkPrompt << "```" + chunk.toObject()["fileName"].toString();
        chunkPrompt << chunk.toObject()["content"].toString();
        chunkPrompt << "```\n";
    }

    return chunkPrompt.join('\n');
}

QString Utils::createCMakePrompt(const QString &testFramework)
{
    QString prompt("根据上面提供的CMakeList.txt文件内容，帮我为单元测试创建CMakeList.txt文件\n\n"
                   "关键原则：\n"
                   "- 测试框架为%1\n"
                   "- 需要判断提供的CMakeList文件是聚合型CMakeLists还是构建型CMakeLists\n"
                   "- 只需要生成一个CMakeLists.txt文件，不需要生成多个\n\n"
                   "根据下面的规则来判断提供的CMakeLists.txt文件是聚合型CMakeLists还是构建型CMakeLists：\n"
                   "- 如果CMakeLists.txt文件中包含add_subdirectory命令，则为聚合型CMakeLists\n"
                   "- 如果CMakeLists.txt文件中包含add_executable或add_library命令，则为构建型CMakeLists\n\n"
                   "为聚合型CMakeLists创建单元测试的CMakeLists.txt文件要求：\n"
                   "- 只需要获取提供的CMakeLists.txt文件中所有的子目录并添加\n\n"
                   "为构建型CMakeLists创建单元测试的CMakeLists.txt文件要求：\n"
                   "- 单元测试源文件采用下面提供的风格来获取：：\n"
                   "```cmake\nFILE(GLOB UT_FILES\n"
                   "    \"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp\"\n"
                   "    \"${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp\"\n)\n"
                   "```\n"
                   "- 根据提供的CMakeLists.txt文件内容，添加必要的依赖");
    return prompt.arg(testFramework);
}

QStringList Utils::queryCodePart(const QString &content, const QString &type)
{
    static QString regexFormat(R"(```%1\n((.*\n)*?.*)\n```)");
    QRegularExpression regex(regexFormat.arg(type));
    QRegularExpressionMatchIterator it = regex.globalMatch(content);
    QStringList matches;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        matches << match.captured(1);
    }

    return matches;
}

bool Utils::checkAnyState(NodeItem *item, ItemState state)
{
    if (item->itemNode->asFileNode()) {
        return item->state == state;
    } else if (item->hasChildren()) {
        for (int i = 0; i < item->rowCount(); ++i) {
            if (checkAnyState(dynamic_cast<NodeItem *>(item->child(i)), state))
                return true;
        }
    }
    return false;
}

bool Utils::checkAllState(NodeItem *item, ItemState state)
{
    if (item->itemNode->asFileNode()) {
        return item->state == state;
    } else if (item->hasChildren()) {
        for (int i = 0; i < item->rowCount(); ++i) {
            if (!checkAllState(dynamic_cast<NodeItem *>(item->child(i)), state))
                return false;
        }
        return true;
    }
    return false;
}
