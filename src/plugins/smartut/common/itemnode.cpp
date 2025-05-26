// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemnode.h"
#include "utils/utils.h"

#include "common/util/qtcassert.h"
#include "services/project/projectservice.h"

#include <DFileIconProvider>

#include <QFileInfo>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

ProjectNode *Node::parentProjectNode() const
{
    if (!nodeParentFolderNode)
        return nullptr;
    auto pn = nodeParentFolderNode->asProjectNode();
    if (pn)
        return pn;
    return nodeParentFolderNode->parentProjectNode();
}

FolderNode *Node::parentFolderNode() const
{
    return nodeParentFolderNode;
}

dpfservice::ProjectInfo Node::projectInfo() const
{
    auto prjSrv = dpfGetService(ProjectService);
    Q_ASSERT(prjSrv);

    const auto &allInfos = prjSrv->getAllProjectInfo();
    auto iter = std::find_if(allInfos.cbegin(), allInfos.cend(),
                             [this](const ProjectInfo &info) {
                                 return nodeFilePath.startsWith(info.workspaceFolder());
                             });

    return iter == allInfos.cend() ? ProjectInfo() : *iter;
}

QString Node::filePath() const
{
    return nodeFilePath;
}

QString Node::displayName() const
{
    return QFileInfo(nodeFilePath).fileName();
}

QString Node::tooltip() const
{
    return nodeFilePath;
}

QIcon Node::icon() const
{
    return DFileIconProvider::globalProvider()->icon(QFileInfo(nodeFilePath));
}

bool Node::sortByPath(const Node *a, const Node *b)
{
    return a->filePath() < b->filePath();
}

void Node::setParentFolderNode(FolderNode *parentFolder)
{
    nodeParentFolderNode = parentFolder;
}

void Node::setFilePath(const QString &filePath)
{
    nodeFilePath = filePath;
}

FileNode::FileNode(const QString &filePath)
{
    setFilePath(filePath);
}

void FileNode::setSourceFiles(const QStringList &files)
{
    sourceList = files;
}

QStringList FileNode::sourceFiles() const
{
    return sourceList;
}

FolderNode::FolderNode(const QString &folderPath)
{
    setFilePath(folderPath);
    nodeDisplayName = Node::displayName();
}

void FolderNode::setDisplayName(const QString &name)
{
    nodeDisplayName = name;
}

QString FolderNode::displayName() const
{
    return nodeDisplayName;
}

void FolderNode::addNode(std::unique_ptr<Node> &&node)
{
    QTC_ASSERT(node, return );
    QTC_ASSERT(!node->parentFolderNode(), qDebug("Node has already a parent folder"));
    node->setParentFolderNode(this);
    children.emplace_back(std::move(node));
}

const QList<Node *> FolderNode::nodes() const
{
    QList<Node *> nodeList;
    std::transform(children.begin(), children.end(), std::back_inserter(nodeList),
                   [](const auto &pointer) {
                       return pointer.get();
                   });
    return nodeList;
}

FolderNode *FolderNode::folderNode(const QString &directory) const
{
    auto iter = std::find_if(children.cbegin(), children.cend(),
                             [directory](const std::unique_ptr<Node> &n) {
                                 FolderNode *fn = n->asFolderNode();
                                 return fn && fn->filePath() == directory;
                             });

    return iter == children.cend() ? nullptr : static_cast<FolderNode *>(iter->get());
}

Node *FolderNode::findNode(const std::function<bool(Node *)> &filter)
{
    if (filter(this))
        return this;

    for (const std::unique_ptr<Node> &n : children) {
        if (n->asFileNode() && filter(n.get())) {
            return n.get();
        } else if (FolderNode *folder = n->asFolderNode()) {
            Node *result = folder->findNode(filter);
            if (result)
                return result;
        }
    }
    return nullptr;
}

FolderNode *FolderNode::findChildFolderNode(const std::function<bool(FolderNode *)> &predicate) const
{
    for (const std::unique_ptr<Node> &n : children) {
        if (FolderNode *fn = n->asFolderNode())
            if (predicate(fn))
                return fn;
    }
    return nullptr;
}

void FolderNode::addNestedNodes(std::vector<std::unique_ptr<FileNode>> &&files,
                                const QString &workspace,
                                const FolderNodeFactory &factory)
{
    using DirWithNodes = std::pair<QString, std::vector<std::unique_ptr<FileNode>>>;
    std::vector<DirWithNodes> fileNodesPerDir;
    for (auto &f : files) {
        if (!f->filePath().startsWith(workspace))
            continue;

        QFileInfo fileInfo(f->filePath());
        const QString parentDir = fileInfo.absolutePath();
        const auto it = std::lower_bound(fileNodesPerDir.begin(), fileNodesPerDir.end(), parentDir,
                                         [](const DirWithNodes &nad, const QString &dir) { return nad.first < dir; });
        if (it != fileNodesPerDir.end() && it->first == parentDir) {
            it->second.emplace_back(std::move(f));
        } else {
            DirWithNodes dirWithNodes;
            dirWithNodes.first = parentDir;
            dirWithNodes.second.emplace_back(std::move(f));
            fileNodesPerDir.insert(it, std::move(dirWithNodes));
        }
    }

    for (DirWithNodes &dirWithNodes : fileNodesPerDir) {
        FolderNode *const folderNode = Utils::recursiveFindOrCreateFolderNode(this, dirWithNodes.first,
                                                                              workspace, factory);
        for (auto &f : dirWithNodes.second)
            folderNode->addNode(std::move(f));
    }
}

QIcon FolderNode::icon() const
{
    if (!QFile::exists(filePath()))
        return QIcon::fromTheme("folder");

    return Node::icon();
}

VirtualFolderNode::VirtualFolderNode(const QString &folderPath)
    : FolderNode(folderPath)
{
    setFilePath(folderPath);
}

ProjectNode::ProjectNode(const QString &projectFilePath)
    : FolderNode(projectFilePath)
{
    setFilePath(projectFilePath);
}
