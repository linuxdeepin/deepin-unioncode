// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectgenerator.h"
#include "projectservice.h"
#include "window/windowservice.h"
#include "window/windowelement.h"

#include <QFileDialog>

/*!
 * \brief The ProjectGenerator class 工程生成器对象
 * 该类主要功能为生成各类工程提供接口, 该类与ProjectService接口分开
 * 达到接口隔离和适应插件组合变化
 * 模式1. 仅仅包含ProjectGenerator的插件（独立功能的插件）
 * 模式2. 包含ProjectGenerator、主界面框架组件发布、流程执行（倾向于业务流程）
 * ProjectService 面向工程核心(ProjectCore) 提供对外接口
 * ProjectGenerator 面向工程扩展实现其他插件对外的接口
 * ProjectCore <-> ProjectService <-> [outside plugin]
 */


QStringList dpfservice::ProjectGenerator::supportLanguages()
{
    return {};
}

QStringList dpfservice::ProjectGenerator::supportFileNames()
{
    return {};
}

QAction *dpfservice::ProjectGenerator::openProjectAction(const QString &language,
                                                         const QString &actionText)
{
    auto result = new QAction(actionText);
    QObject::connect(result, &QAction::triggered, [=](){
        QString iniPath = CustomPaths::user(CustomPaths::Flags::Configures)
                + QDir::separator() + QString("project_record.support");
        QSettings setting(iniPath, QSettings::IniFormat);
        QString lastPath = setting.value(language + "-" + actionText).toString();
        if (lastPath.isEmpty()) {
            lastPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        }

        QFileDialog fileDialog;
        fileDialog.setFileMode(QFileDialog::Directory);
        fileDialog.setOption(QFileDialog::DontResolveSymlinks);
        fileDialog.setWindowTitle(QFileDialog::tr("Open %0 Project Directory").arg(language));
        fileDialog.setDirectory(lastPath);
        fileDialog.setWindowFlags(fileDialog.windowFlags() | Qt::WindowStaysOnTopHint);
        if (fileDialog.exec() == QDialog::Accepted) {
            QString workspace = fileDialog.selectedUrls().first().path();
            setting.setValue(language + "-" + actionText, workspace); // save open history
            if (canOpenProject(actionText, language, workspace))
                doProjectOpen(language, actionText, workspace);
        }
    });
    return result;
}

/*!
 * \brief dpfservice::ProjectGenerator::projectIsOpened
 *  check project is opened, default can't reopen opened project
 * \param kitName project kitName
 * \param language project language
 * \param projectPath to open project path
 * \return is opened return true, else return false;
 */
bool dpfservice::ProjectGenerator::canOpenProject(const QString &kitName,
                                                  const QString &language,
                                                  const QString &workspace)
{
    if (isOpenedProject(kitName, language, workspace))
        return false;

    QStringList fileNames = supportFileNames();
    if (!fileNames.isEmpty()) {
        for (auto filename : fileNames) {
            if (QDir(workspace).exists(filename))
                return true;
        }
        ContextDialog::ok(QDialog::tr("Cannot open the project!\n"
                                      "not exists support files: %0")
                          .arg(supportFileNames().join(",")));
        return false;
    }
    return true;
}

bool dpfservice::ProjectGenerator::isOpenedProject(const QString &kitName,
                                                   const QString &language,
                                                   const QString &workspace)
{
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    auto proInfos = projectService->getAllProjectInfo();
    for (auto info : proInfos) {
        if (info.kitName() == kitName
                && info.language() == language
                && info.workspaceFolder() == workspace) {
            return true;
        }
    }
    return false;
}

void dpfservice::ProjectGenerator::doProjectOpen(const QString &language,
                                                 const QString &actionText,
                                                 const QString &workspace)
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());

    if (!projectService)
        return;

    auto generator = projectService->createGenerator<ProjectGenerator>(actionText);
    if (!generator)
        return;

    auto widget = generator->configureWidget(language, workspace);
    if (widget) {
        widget->exec();
    }
}

/*!
 * \brief configure 生成器配置界面，子类需要重载实现
 *  按照一般的做法，当前界面应当生成配置文件与工程路径绑定，
 *  在界面上选中、输入信息存入配置文件
 * \param projectPath 工程路径
 * \return 返回工程配置界面
 */
QDialog *dpfservice::ProjectGenerator::configureWidget(const QString &language, const QString &workspace)
{
    Q_UNUSED(language)
    Q_UNUSED(workspace)
    return nullptr;
}

/*!
 * \brief configure 执行生成器的过程
 *  按照生成器配置界面约定规则，实现执行过程，
 *  例如在cmake中，需要执行configure指令生成cache等相关文件
 *  执行该函数，应当首先确保前置条件满足(配置文件已生成)。
 * \param projectPath 工程路径
 * \return 过程执行结果
 */
bool dpfservice::ProjectGenerator::configure(const dpfservice::ProjectInfo &projectInfo) {

    //  "filePath", "kitName", "language", "workspace"
    recent.saveOpenedProject(projectInfo.kitName(),
                             projectInfo.language(),
                             projectInfo.workspaceFolder());
    navigation.doSwitch(MWNA_EDIT);

    editor.switchWorkspace(MWCWT_PROJECTS);

    if (!projectInfo.workspaceFolder().isEmpty()) {
        dpf::Event event;
        event.setTopic(T_COLLABORATORS);
        event.setData(D_OPEN_REPOS);
        event.setProperty(P_WORKSPACEFOLDER, projectInfo.workspaceFolder());
        dpf::EventCallProxy::instance().pubEvent(event);
    }

    Generator::started(); // emit starded

    return false;
}

/*!
 * \brief createRootItem 创建文件树路径，子类需要重载实现
 *  执行该函数应当首先确定前置条件的满足，比如已经执行了生成器的过程。
 * \param ProjectInfo 工程信息
 * \return
 */
QStandardItem *dpfservice::ProjectGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    QIcon icon = CustomIcons::icon(info.workspaceFolder());
    QString displyName = QFileInfo(info.workspaceFolder()).fileName();
    QString tooltip = info.workspaceFolder();
    auto rootItem = new QStandardItem(icon, displyName);
    rootItem->setToolTip(tooltip);
    return rootItem;
}

/*!
 * \brief removeRootItem 删除文件树根节点，子类需要重载实现
 *  执行函数如果存在异步加载的情况，则需要通过该接口实现异步释放
 * \param info
 */
void dpfservice::ProjectGenerator::removeRootItem(QStandardItem *root)
{
    Q_UNUSED(root)
}

/*!
 * \brief createIndexMenu 文件树index点击时触发，子类需要重载实现
 *  文件树中右键触发的创建Menu，传入树中QModelIndex，它与Item数据相通(Qt规范)
 *  QMenu对象在插件使用方会被释放
 * \param index 索引数据
 * \return
 */
QMenu *dpfservice::ProjectGenerator::createItemMenu(const QStandardItem *item)
{
    Q_UNUSED(item)
    return nullptr;
}

void dpfservice::ProjectGenerator::createDocument(const QStandardItem *item, const QString &filePath)
{
    Q_UNUSED(item)
    QString workspace, language;
    auto root = ProjectGenerator::root(const_cast<QStandardItem *>(item));
    if (root) {
        auto rootInfo = dpfservice::ProjectInfo::get(root);
        workspace = rootInfo.workspaceFolder();
        language = rootInfo.language();
    }

    QFile file(filePath);
    if (file.open(QFile::OpenModeFlag::NewOnly)) {
        file.close();
    }

    editor.openFileWithKey(workspace, language, filePath);
}

/*!
 * \brief root 获取子节点的根节点
 * \param child 子节点
 * \return 根节点
 */
QStandardItem *dpfservice::ProjectGenerator::root(QStandardItem *child)
{
    if (!child)
        return nullptr;
    QStandardItem *parent = child->parent();
    if (parent)
        return root(parent);
    return child;
}

/*!
 * \brief root 获取子节点的根节点
 * \param child 子节点
 * \return 根节点
 */
const QModelIndex dpfservice::ProjectGenerator::root(const QModelIndex &child)
{
    if (!child.isValid())
        return {};
    const QModelIndex parent = child.parent();
    if (parent.isValid())
        return root(parent);
    return child;
}
