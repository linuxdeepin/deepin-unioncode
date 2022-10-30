/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef PROJECTGENERATOR_H
#define PROJECTGENERATOR_H

#include "projectinfo.h"

#include "common/common.h"

#include <framework/framework.h>

#include <QStandardItem>
#include <QMenu>

namespace dpfservice {

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
class ProjectGenerator : public Generator
{
    Q_OBJECT
public:

    ProjectGenerator(){}

    /*!
     * \brief configure 生成器配置界面，子类需要重载实现
     *  按照一般的做法，当前界面应当生成配置文件与工程路径绑定，
     *  在界面上选中、输入信息存入配置文件
     * \param projectPath 工程路径
     * \return 返回工程配置界面
     */
    virtual QDialog* configureWidget(const QString &language, const QString &projectPath) {
        Q_UNUSED(language)
        Q_UNUSED(projectPath)
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
    virtual bool configure(const ProjectInfo &projectInfo) {

        //  "filePath", "kitName", "language", "workspace"
        recent.saveOpenedProject({projectInfo.projectFilePath(),
                                  projectInfo.kitName(),
                                  projectInfo.language(),
                                  projectInfo.workspaceFolder()});

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
    virtual QStandardItem *createRootItem(const ProjectInfo &info) {
        Q_UNUSED(info)
        return nullptr;
    }

    /*!
     * \brief removeRootItem 删除文件树根节点，子类需要重载实现
     *  执行函数如果存在异步加载的情况，则需要通过该接口实现异步释放
     * \param info
     */
    virtual void removeRootItem(QStandardItem *root) {
        Q_UNUSED(root)
    }

    /*!
     * \brief createIndexMenu 文件树index点击时触发，子类需要重载实现
     *  文件树中右键触发的创建Menu，传入树中QModelIndex，它与Item数据相通(Qt规范)
     *  QMenu对象在插件使用方会被释放
     * \param index 索引数据
     * \return
     */
    virtual QMenu* createItemMenu(const QStandardItem *item) {
        Q_UNUSED(item)
        return nullptr;
    }

    /*!
     * \brief root 获取子节点的根节点
     * \param child 子节点
     * \return 根节点
     */
    inline static QStandardItem *root(QStandardItem *child)
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
    static const QModelIndex root(const QModelIndex &child)
    {
        if (!child.isValid())
            return {};
        const QModelIndex parent = child.parent();
        if (parent.isValid())
            return root(parent);
        return child;
    }

protected:
    template<class T> struct ParseInfo
    {
        T result;
        bool isNormal = true;
    };
};
} // namespace dpfservice

#endif // PROJECTGENERATOR_H
