// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projecttree.h"
#include "projectinfodialog.h"
#include "projectselectionmodel.h"
#include "projectdelegate.h"
#include "projectmodel.h"
#include "transceiver/sendevents.h"
#include "common/common.h"
#include "services/project/projectservice.h"
#include "services/window/windowelement.h"
#include "services/terminal/terminalservice.h"

#include <DTreeView>
#include <DPushButton>
#include <DLineEdit>
#include <DDialog>

#include <QDebug>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QPushButton>
#include <QDrag>
#include <QApplication>
#include <QUrl>
#include <QClipboard>
#include <QDesktopServices>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

class ProjectTreePrivate
{
    friend class ProjectTree;
    ProjectModel *itemModel { nullptr };
    ProjectSelectionModel *sectionModel { nullptr };
    ProjectDelegate *delegate { nullptr };
    DDialog *messDialog = nullptr;
    QPoint startPos;
    QString currentFile = "";
    bool autoFocusState = true;
    int itemDepth(const QStandardItem *item)
    {
        int depth = 0;
        const QStandardItem *current = item;
        while (current->parent()) {
            current = current->parent();
            depth++;
        }
        return depth;
    }
};

ProjectTree::ProjectTree(QWidget *parent)
    : DTreeView(parent), d(new ProjectTreePrivate)
{
    setLineWidth(0);
    setContentsMargins(0, 0, 0, 0);
    setFrameShape(QFrame::NoFrame);
    DStyle::setFrameRadius(this, 0);
    setIconSize(QSize(16, 16));

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setTextElideMode(Qt::ElideNone);
    setEditTriggers(DTreeView::NoEditTriggers);   //节点不能编辑
    setSelectionBehavior(DTreeView::SelectRows);   //一次选中整行
    setSelectionMode(DTreeView::SingleSelection);   //单选，配合上面的整行就是一次选单行
    setHeaderHidden(true);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    header()->setStretchLastSection(false);

    d->itemModel = new ProjectModel(this);
    setModel(d->itemModel);

    // 右键菜单创建
    QObject::connect(this, &ProjectTree::itemMenuRequest,
                     this, &ProjectTree::doItemMenuRequest);

    // 双击操作
    QObject::connect(this, &ProjectTree::doubleClicked,
                     this, &ProjectTree::doDoubleClicked);

    QObject::connect(this, &ProjectTree::expanded,
                     this, [=](const QModelIndex &index) { SendEvents::projectNodeExpanded(index); });

    QObject::connect(this, &ProjectTree::collapsed,
                     this, [=](const QModelIndex &index) { SendEvents::projectNodeCollapsed(index); });

    d->sectionModel = new ProjectSelectionModel(d->itemModel);
    setSelectionModel(d->sectionModel);

    d->delegate = new ProjectDelegate(this);
    setItemDelegate(d->delegate);
    this->setDragEnabled(true);

    setProperty("canExpand", true);
}

ProjectTree::~ProjectTree()
{
    if (d) {
        delete d;
    }
}

void ProjectTree::activeProjectInfo(const ProjectInfo &info)
{
    int rowCount = d->itemModel->rowCount();
    for (int currRow = 0; currRow < rowCount; currRow++) {
        auto currItem = d->itemModel->item(currRow, 0);
        if (currItem) {
            auto currInfo = ProjectInfo::get(ProjectGenerator::root(currItem));
            if (currInfo.language() == info.language()
                && currInfo.workspaceFolder() == info.workspaceFolder()
                && currInfo.kitName() == info.kitName()) {
                doActiveProject(currItem);
            }
        }
    }
}

void ProjectTree::activeProjectInfo(const QString &kitName,
                                    const QString &language,
                                    const QString &workspace)
{
    int rowCount = d->itemModel->rowCount();
    for (int currRow = 0; currRow < rowCount; currRow++) {
        auto currItem = d->itemModel->item(currRow, 0);
        if (currItem) {
            auto currInfo = ProjectInfo::get(ProjectGenerator::root(currItem));
            if (currInfo.language() == language
                && currInfo.workspaceFolder() == workspace
                && currInfo.kitName() == kitName) {
                doActiveProject(currItem);
            }
        }
    }
}

void ProjectTree::appendRootItem(QStandardItem *root)
{
    if (!root)
        return;

    // 发送工程创建信号
    using namespace dpfservice;
    auto info = ProjectInfo::get(ProjectGenerator::root(root));

    // 添加工程节点
    QStandardItemModel *model = static_cast<QStandardItemModel *>(DTreeView::model());
    if (model)
        model->appendRow(root);

    if (root->data(Parsing_State_Role).value<ParsingState>() != ParsingState::Done)   //avoid appent root item after complete parse
        root->setData(ParsingState::Wait, Parsing_State_Role);

    // 发送工程节点已创建信号
    SendEvents::projectCreated(info);

    // 激活当前工程节点
    doActiveProject(root);
}

void ProjectTree::removeRootItem(QStandardItem *root)
{
    this->viewport()->setUpdatesEnabled(false);

    ProjectInfo info = ProjectInfo::get(ProjectGenerator::root(root));

    // 从Model中移除
    this->takeRootItem(root);

    // 从生成器中删除
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService)
        return;

    auto generator = projectService->createGenerator<ProjectGenerator>(info.kitName());
    if (generator)
        generator->removeRootItem(root);

    // 发送工程删除信号
    SendEvents::projectDeleted(info);

    // 始终保持首选项
    int rowCount = d->itemModel->rowCount();
    if (0 < rowCount) {   // 存在其他工程时
        auto index = d->itemModel->index(0, 0);
        doActiveProject(d->itemModel->itemFromIndex(index));
    }

    this->viewport()->setUpdatesEnabled(true);
}

void ProjectTree::takeRootItem(QStandardItem *root)
{
    // 从展示的模型中删除
    QModelIndex index = d->itemModel->indexFromItem(root);
    d->itemModel->takeRow(index.row());
    emit itemDeleted(root);

    if (d->itemModel->rowCount() == 0)
        d->delegate->hideSpinner();
}

void ProjectTree::doItemMenuRequest(QStandardItem *item, QContextMenuEvent *event)
{
    if (!item)
        return;

    auto rootItem = ProjectGenerator::root(item);
    DMenu *menu = nullptr;

    if (rootItem == item) {
        menu = rootMenu(rootItem);
        // add action that running.
        auto runCommand = ActionManager::instance()->command("Debug.Running");
        if (runCommand && runCommand->action()) {
            menu->addSeparator();
            menu->addAction(runCommand->action());
        }
        QFileInfo info(item->data(Qt::ToolTipRole).toString());
        if (info.isDir()) {
            menu->addSeparator();
            QAction *newDocAction = new QAction(tr("New Document"), this);
            QObject::connect(newDocAction, &QAction::triggered, this, [=]() {
                actionNewDocument(item);
            });
            QAction *newDirAction = new QAction(tr("New Directory"), this);
            QObject::connect(newDirAction, &QAction::triggered, this, [=]() {
                actionNewDirectory(item);
            });
            menu->addAction(newDocAction);
            menu->addAction(newDirAction);
        }
    } else {
        menu = childMenu(rootItem, item);
    }

    // add action that show contain folder.
    menu->addSeparator();
    QAction *showContainFolder = new QAction(tr("Show Containing Folder"), this);
    connect(showContainFolder, &QAction::triggered, [=]() {
        QString filePath = item->toolTip();
        QFileInfo info(filePath);
        QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
    });
    menu->addAction(showContainFolder);

    connect(this, &ProjectTree::itemDeleted, menu, [=](QStandardItem *deletedItem) {
        if (item == deletedItem || ProjectGenerator::root(item) == deletedItem)
            menu->close();
    });

    if (menu) {
        menu->move(event->globalPos());
        menu->exec();
        delete menu;
    }
}

void ProjectTree::expandedProjectDepth(const QStandardItem *root, int depth)
{
    if (!root)
        return;

    if (d->itemDepth(root) < depth) {   //满足深度
        expand(d->itemModel->indexFromItem(root));
        for (int i = 0; i < root->rowCount(); i++) {
            QStandardItem *childitem = root->child(i);
            if (root->hasChildren()) {
                expandedProjectDepth(childitem, depth);
            }
        }
    }
}

void ProjectTree::expandedProjectAll(const QStandardItem *root)
{
    if (!root)
        return;

    expand(d->itemModel->indexFromItem(root));
    if (root->hasChildren()) {
        for (int i = 0; i < root->rowCount(); i++) {
            QStandardItem *childitem = root->child(i);
            expandedProjectAll(childitem);
        }
    }
}

void ProjectTree::selectProjectFile(const QString &file)
{
    d->currentFile = file;
    if (!d->autoFocusState)
        return;
    focusCurrentFile();
}

void ProjectTree::expandItemByFile(const QStringList &filePaths)
{
    QModelIndex root = d->itemModel->index(0, 0);

    if (!root.isValid()) {
        return;
    }
    for (auto filePath : filePaths) {
        // hints = 2 : project`s root and lib-node might use the same filePath
        QModelIndexList indices = model()->match(root, Qt::ToolTipRole, filePath, 2, Qt::MatchExactly | Qt::MatchRecursive);
        if (!indices.isEmpty()) {
            for (auto index : indices)
                expand(index);
        }
    }
}

void ProjectTree::focusCurrentFile()
{
    QModelIndex root = d->itemModel->index(0, 0);
    if (!root.isValid()) {
        return;
    }
    if (d->currentFile.isEmpty()) {
        clearSelection();
        return;
    }
    QModelIndexList indices = model()->match(root, Qt::ToolTipRole, d->currentFile, 1, Qt::MatchExactly | Qt::MatchRecursive);
    if (!indices.isEmpty()) {
        QModelIndex index = indices.first();
        setCurrentIndex(index);
    }
}

void ProjectTree::setAutoFocusState(bool state)
{
    d->autoFocusState = state;
}

bool ProjectTree::getAutoFocusState() const
{
    return d->autoFocusState;
}

QList<dpfservice::ProjectInfo> ProjectTree::getAllProjectInfo()
{
    using namespace dpfservice;
    QList<ProjectInfo> result;
    for (int row = 0; row < d->itemModel->rowCount(); row++) {
        result << ProjectInfo::get(d->itemModel->index(row, 0));
    }
    return result;
}

ProjectInfo ProjectTree::getProjectInfo(const QString &kitName, const QString &workspace) const
{
    ProjectInfo projectInfo;
    for (int row = 0; row < d->itemModel->rowCount(); row++) {
        ProjectInfo info = ProjectInfo::get(d->itemModel->index(row, 0));
        if (kitName == info.kitName() && workspace == info.workspaceFolder()) {
            projectInfo = info;
            break;
        }
    }
    return projectInfo;
}

QStandardItem *ProjectTree::getActiveProjectItem() const
{
    return d->itemModel->itemFromIndex(d->delegate->getActiveProject());
}

ProjectInfo ProjectTree::getActiveProjectInfo() const
{
    ProjectInfo projectInfo;
    auto activeProject = d->delegate->getActiveProject();
    if (activeProject.isValid()) {
        projectInfo = ProjectInfo::get(activeProject);
    }
    return projectInfo;
}

bool ProjectTree::updateProjectInfo(ProjectInfo &projectInfo)
{
    for (int row = 0; row < d->itemModel->rowCount(); row++) {
        QModelIndex modelIndex = d->itemModel->index(row, 0);
        ProjectInfo innerInfo = ProjectInfo::get(modelIndex);
        if (projectInfo.isSame(innerInfo)) {
            ProjectInfo::set(d->itemModel->itemFromIndex(modelIndex), projectInfo);
            return true;
        }
    }

    return false;
}

bool ProjectTree::hasProjectInfo(const ProjectInfo &info) const
{
    ProjectInfo projectInfo = getProjectInfo(info.kitName(), info.workspaceFolder());
    return !projectInfo.isEmpty();
}

void ProjectTree::contextMenuEvent(QContextMenuEvent *event)
{
    DTreeView::contextMenuEvent(event);
    QModelIndex index = indexAt(event->pos());
    selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
    itemMenuRequest(d->itemModel->itemFromIndex(index), event);
}

void ProjectTree::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        d->startPos = event->pos();
    DTreeView::mousePressEvent(event);
}

void ProjectTree::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        int distance = (event->pos() - d->startPos).manhattanLength();
        if (distance >= QApplication::startDragDistance())
            performDrag();
    }
    DTreeView::mouseMoveEvent(event);
}

void ProjectTree::resizeEvent(QResizeEvent *event)
{
    const int columns = header()->count();
    const int minimumWidth = columns > 1 ? viewport()->width() / columns
                                         : viewport()->width();
    header()->setMinimumSectionSize(minimumWidth);
    DTreeView::resizeEvent(event);
}

DMenu *ProjectTree::childMenu(const QStandardItem *root, QStandardItem *childItem)
{
    DMenu *menu = nullptr;
    QString toolKitName = ProjectInfo::get(root).kitName();
    // 获取支持右键菜单生成器
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService->supportGeneratorName<ProjectGenerator>().contains(toolKitName)) {
        menu = projectService->createGenerator<ProjectGenerator>(toolKitName)->createItemMenu(childItem);
    }

    if (!menu)
        menu = new DMenu();

    QAction *newDirAction = new QAction(tr("New Directory"), this);
    QObject::connect(newDirAction, &QAction::triggered, this, [=]() {
        actionNewDirectory(childItem);
    });

    QAction *newDocAction = new QAction(tr("New Document"), this);
    QObject::connect(newDocAction, &QAction::triggered, this, [=]() {
        actionNewDocument(childItem);
    });

    QAction *renameDocAction = new QAction(tr("Rename"), this);
    QObject::connect(renameDocAction, &QAction::triggered, this, [=]() {
        actionRenameDocument(childItem);
    });

    QModelIndex index = d->itemModel->indexFromItem(childItem);
    QFileInfo info(index.data(Qt::ToolTipRole).toString());

    // add open in terminal menu item.
    QAction *openInTerminal = new QAction(tr("Open In Terminal"), this);
    menu->addAction(openInTerminal);
    connect(openInTerminal, &QAction::triggered, [=]() {
        actionOpenInTerminal(childItem);
    });

    if (info.isDir()) {
        menu->addAction(newDocAction);
        menu->addAction(newDirAction);
    }

    if (info.isFile()) {
        newDocAction->setEnabled(false);
        // add delete file menu item.
        QAction *deleteDocAction = new QAction(tr("Delete Document"), this);
        QObject::connect(deleteDocAction, &QAction::triggered, this, [=]() {
            actionDeleteDocument(childItem);
        });
        deleteDocAction->setEnabled(true);

        menu->addAction(deleteDocAction);
        menu->addAction(renameDocAction);
    }

    return menu;
}

QMenu *ProjectTree::rootMenu(QStandardItem *root)
{
    DMenu *menu = nullptr;
    QString toolKitName = ProjectInfo::get(root).kitName();
    // 获取支持右键菜单生成器
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService->supportGeneratorName<ProjectGenerator>().contains(toolKitName)) {
        menu = projectService->createGenerator<ProjectGenerator>(toolKitName)->createItemMenu(root);
    }
    if (!menu)
        menu = new DMenu();

    QAction *activeProjectAction = new QAction(QAction::tr("Project Active"), menu);
    QAction *closeAction = new QAction(QAction::tr("Project Close"), menu);
    QAction *propertyAction = new QAction(QAction::tr("Project Info"), menu);
    QObject::connect(activeProjectAction, &QAction::triggered, activeProjectAction, [=]() { doActiveProject(root); });
    QObject::connect(closeAction, &QAction::triggered, closeAction, [=]() { doCloseProject(root); });
    QObject::connect(propertyAction, &QAction::triggered, propertyAction, [=]() { doShowProjectInfo(root); });
    menu->insertAction(nullptr, activeProjectAction);
    menu->insertAction(nullptr, closeAction);
    menu->insertAction(nullptr, propertyAction);
    return menu;
}

void ProjectTree::performDrag()
{
    QModelIndex index = currentIndex();
    QStandardItem *item = d->itemModel->itemFromIndex(index);
    if (item) {
        QMimeData *mimeData = new QMimeData;
        QList<QUrl> urls;
        QString filePath = "file:" + index.data(Qt::ToolTipRole).toString();
        urls.append(QUrl(filePath));
        mimeData->setUrls(urls);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->exec();
    }
}

void ProjectTree::itemModified(QStandardItem *item, const QList<QStandardItem *> &childs)
{
    setUpdatesEnabled(false);
    auto parentIndex = d->itemModel->indexFromItem(item);
    int childCount = d->itemModel->rowCount(parentIndex);
    d->itemModel->removeRows(0, childCount, parentIndex);
    item->appendRows(childs);
    setUpdatesEnabled(true);
}

void ProjectTree::doDoubleClicked(const QModelIndex &index)
{
    QFileInfo info(index.data(Qt::ToolTipRole).toString());
    if (info.exists() && info.isFile()) {
        QString workspaceFolder, language;
        QModelIndex rootIndex = ProjectGenerator::root(index);
        if (rootIndex.isValid()) {
            auto info = ProjectInfo::get(rootIndex);
            workspaceFolder = info.workspaceFolder();
            language = info.language();
        }
        editor.openFile(workspaceFolder, info.filePath());
    }
}

void ProjectTree::doCloseProject(QStandardItem *root)
{
    if (!root && root != ProjectGenerator::root(root))
        return;

    this->removeRootItem(root);
}

void ProjectTree::doActiveProject(QStandardItem *root)
{
    if (!root && root != ProjectGenerator::root(root))
        return;
    d->delegate->setActiveProject(d->itemModel->indexFromItem(root));
    SendEvents::projectActived(ProjectInfo::get(root));
}

void ProjectTree::actionRenameDocument(const QStandardItem *item)
{
    auto dialog = new DDialog(this);
    auto inputEdit = new DLineEdit(dialog);

    inputEdit->setPlaceholderText(tr("New Document Name"));
    inputEdit->lineEdit()->setAlignment(Qt::AlignLeft);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Rename"));

    dialog->addContent(inputEdit);
    dialog->addButton(tr("Ok"), true, DDialog::ButtonRecommend);

    QObject::connect(dialog, &DDialog::buttonClicked, dialog, [=]() {
        if (!inputEdit->text().isEmpty()) {
            renameDocument(item, inputEdit->text());
        }
        dialog->close();
    });

    dialog->exec();
}

void ProjectTree::renameDocument(const QStandardItem *item, const QString &newFileName)
{
    QModelIndex index = d->itemModel->indexFromItem(item);
    QFileInfo info(index.data(Qt::ToolTipRole).toString());

    auto root = ProjectGenerator::root(const_cast<QStandardItem *>(item));
    QString toolKitName = ProjectInfo::get(root).kitName();

    QString oldFilePath = info.absoluteFilePath();
    QString newFilePath;

    if (info.isDir()) {
        newFilePath = info.filePath() + QDir::separator() + newFileName;
    } else if (info.isFile()) {
        newFilePath = info.path() + QDir::separator() + newFileName;
    }

    if (QFile::exists(newFilePath)) {
        bool doOverWrite = false;
        auto okCallBack = [&]() {
            doOverWrite = true;
        };

        if (d->messDialog != nullptr) {
            d->messDialog->setMessage(tr("A file with name %1 already exists. Would you like to overwrite it?").arg(newFileName));
        } else {
            d->messDialog = new DDialog(this);
            d->messDialog->setIcon(QIcon::fromTheme("dialog-warning"));
            d->messDialog->setMessage(tr("A file with name %1 already exists. Would you like to overwrite it?").arg(newFileName));
            d->messDialog->insertButton(0, tr("Cancel"));
            d->messDialog->insertButton(1, tr("Ok"), true, DDialog::ButtonWarning);
        }

        connect(d->messDialog, &DDialog::buttonClicked, [=](int index) {
            if (index == 0) {
                d->messDialog->reject();
            } else if (index == 1) {
                okCallBack();
                QFile::remove(newFilePath);
                d->messDialog->accept();
            }
        });

        d->messDialog->exec();

        if (!doOverWrite)
            return;
    }
    QFile file(oldFilePath);
    file.rename(newFilePath);
}

void ProjectTree::actionNewDirectory(const QStandardItem *item)
{
    auto dialog = new DDialog(this);
    auto inputEdit = new DLineEdit(dialog);

    inputEdit->setPlaceholderText(tr("New Dirctory Name"));
    inputEdit->lineEdit()->setAlignment(Qt::AlignLeft);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("New Dirctory"));

    dialog->addContent(inputEdit);
    dialog->addButton(tr("Ok"), true, DDialog::ButtonRecommend);
    dialog->setOnButtonClickedClose(false);

    QObject::connect(dialog, &DDialog::buttonClicked, dialog, [=]() {
        auto ret = false;
        if (!inputEdit->text().isEmpty()) {
            ret = createNewDirectory(item, inputEdit->text());
        }

        if (ret)
            dialog->close();
    });

    dialog->exec();
}

void ProjectTree::actionNewDocument(const QStandardItem *item)
{
    auto dialog = new DDialog(this);
    auto inputEdit = new DLineEdit(dialog);

    inputEdit->setPlaceholderText(tr("New Document Name"));
    inputEdit->lineEdit()->setAlignment(Qt::AlignLeft);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("New Document"));

    dialog->addContent(inputEdit);
    dialog->addButton(tr("Ok"), true, DDialog::ButtonRecommend);

    QObject::connect(dialog, &DDialog::buttonClicked, dialog, [=]() {
        if (!inputEdit->text().isEmpty()) {
            creatNewDocument(item, inputEdit->text());
        }
        dialog->close();
    });

    dialog->exec();
}

void ProjectTree::actionDeleteDocument(QStandardItem *item)
{
    QModelIndex index = d->itemModel->indexFromItem(item);
    QFileInfo info(index.data(Qt::ToolTipRole).toString());
    if (!info.isFile())
        return;

    QString message = tr("Delete operation not be recoverable, delete anyway?");

    DDialog dialog;
    dialog.setMessage(message);
    dialog.setWindowTitle(tr("Delete: ") + info.fileName());
    dialog.setIcon(QIcon::fromTheme("dialog-warning"));
    dialog.insertButton(0, tr("Ok"));
    dialog.insertButton(1, tr("Cancel"));
    int code = dialog.exec();

    if (code == 1)
        return;

    QFile(info.filePath()).remove();

    // notify file deleted event.
    QStandardItem *root = ProjectGenerator::root(item);
    auto projectInfo = ProjectInfo::get(root);

    project.fileDeleted(info.filePath(), projectInfo.kitName());
}

void ProjectTree::actionOpenInTerminal(const QStandardItem *menuItem)
{
    if (!menuItem)
        return;

    QModelIndex index = d->itemModel->indexFromItem(menuItem);
    QFileInfo fileInfo(index.data(Qt::ToolTipRole).toString());

    QString dirPath;
    if (fileInfo.isFile())
        dirPath = fileInfo.dir().path();
    else if (fileInfo.isDir())
        dirPath = fileInfo.filePath();

    auto terminalService = dpfGetService(TerminalService);
    if (terminalService) {
        terminalService->sendCommand(QString("cd %1\n").arg(dirPath));
        terminalService->sendCommand(QString("clear\n"));
        uiController.switchContext(TERMINAL_TAB_TEXT);
    }
}

bool ProjectTree::createNewDirectory(const QStandardItem *item, const QString &dirName)
{
    QModelIndex index = d->itemModel->indexFromItem(item);
    QFileInfo info(index.data(Qt::ToolTipRole).toString());

    auto root = ProjectGenerator::root(const_cast<QStandardItem *>(item));
    QString toolKitName = ProjectInfo::get(root).kitName();

    QString directoryPath;
    if (info.isDir()) {
        directoryPath = info.filePath() + QDir::separator() + dirName;
    } else if (info.isFile()) {
        directoryPath = info.path() + QDir::separator() + dirName;
    }

    if (QFile::exists(directoryPath)) {
        d->messDialog = new DDialog();
        d->messDialog->setIcon(QIcon::fromTheme("dialog-warning"));
        d->messDialog->setMessage(tr("A directory with name %1 already exists. please reanme it").arg(directoryPath));
        d->messDialog->insertButton(0, tr("Cancel"));
        d->messDialog->insertButton(1, tr("Ok"), true, DDialog::ButtonWarning);

        d->messDialog->exec();
        return false;
    }

    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService->supportGeneratorName<ProjectGenerator>().contains(toolKitName)) {
        projectService->createGenerator<ProjectGenerator>(toolKitName)->createDirectory(item, directoryPath);
    }
    return true;
}

void ProjectTree::creatNewDocument(const QStandardItem *item, const QString &fileName)
{
    QModelIndex index = d->itemModel->indexFromItem(item);
    QFileInfo info(index.data(Qt::ToolTipRole).toString());

    auto root = ProjectGenerator::root(const_cast<QStandardItem *>(item));
    QString toolKitName = ProjectInfo::get(root).kitName();

    QString filePath;
    if (info.isDir()) {
        filePath = info.filePath() + QDir::separator() + fileName;
    } else if (info.isFile()) {
        filePath = info.path() + QDir::separator() + fileName;
    }

    if (QFile::exists(filePath)) {
        bool doOverWrite = false;
        auto okCallBack = [&]() {
            doOverWrite = true;
        };

        d->messDialog = new DDialog();
        d->messDialog->setIcon(QIcon::fromTheme("dialog-warning"));
        d->messDialog->setMessage(tr("A file with name %1 already exists. Would you like to overwrite it?").arg(fileName));
        d->messDialog->insertButton(0, tr("Cancel"));
        d->messDialog->insertButton(1, tr("Ok"), true, DDialog::ButtonWarning);

        connect(d->messDialog, &DDialog::buttonClicked, [=](int index) {
            if (index == 0) {
                d->messDialog->reject();
            } else if (index == 1) {
                okCallBack();
                QFile::remove(filePath);
                d->messDialog->accept();
            }
        });

        d->messDialog->exec();

        if (!doOverWrite)
            return;
    }

    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService->supportGeneratorName<ProjectGenerator>().contains(toolKitName)) {
        projectService->createGenerator<ProjectGenerator>(toolKitName)->createDocument(item, filePath);
    }
}

void ProjectTree::doShowProjectInfo(QStandardItem *root)
{
    if (!root && root != ProjectGenerator::root(root))
        return;

    ProjectInfoDialog dialog;
    QString propertyText = "Language: " + ProjectInfo::get(root).language() + "\n"
            + "KitName: " + ProjectInfo::get(root).kitName() + "\n"
            + "BuildFolder: " + ProjectInfo::get(root).buildFolder() + "\n"
            + "WorkspaceFolder: " + ProjectInfo::get(root).workspaceFolder() + "\n"
            + "BuildType: " + ProjectInfo::get(root).buildType() + "\n"
            + "BuildProgram: " + "\n    " + ProjectInfo::get(root).buildProgram() + "\n"
            + "ConfigCustomArgs: " + "\n    " + ProjectInfo::get(root).configCustomArgs().join(" ") + "\n"
            + "BuildCustomArgs: " + "\n    " + ProjectInfo::get(root).buildCustomArgs().join(" ") + "\n"
            + "CleanCustomArgs: " + "\n    " + ProjectInfo::get(root).cleanCustomArgs().join(" ");
    dialog.setPropertyText(propertyText);
    dialog.exec();
}
