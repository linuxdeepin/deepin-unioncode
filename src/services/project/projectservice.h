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

#ifndef PROJECTSERVICE_H
#define PROJECTSERVICE_H

#include "projectgenerator.h"

namespace dpfservice {

class ProjectService final : public dpf::PluginService,
        dpf::AutoServiceRegister<ProjectService>,
        dpf::QtClassFactory<ProjectGenerator>,
        dpf::QtClassManager<ProjectGenerator>

{
    Q_OBJECT
    Q_DISABLE_COPY(ProjectService)
    typedef dpf::QtClassManager<ProjectGenerator> GeneratorManager;
    typedef dpf::QtClassFactory<ProjectGenerator> GeneratorFactory;
public:
    static QString name();
    explicit ProjectService(QObject *parent = nullptr);

    /*!
     * \brief supportGenerator 獲取支持的工程名稱
     * \return
     */
    QStringList supportGeneratorName()
    {
        return GeneratorFactory::createKeys();
    }

    /*!
     * \brief implGenerator 導入工程文件生成器對象 T = Project::Generator類泛化對象
     * \param name 生成器對象名稱(唯一鍵)
     * \param errorString 錯誤信息
     */
    template<class T = ProjectGenerator>
    bool implGenerator(const QString &name, QString *errorString = nullptr)
    {
        return GeneratorFactory::regClass<T>(name, errorString);
    }

    /*!
     * \brief createGenerator
     * \param name 名稱
     * \param errorString 錯誤信息
     * \return 生成器對象實例
     */
    ProjectGenerator *createGenerator(const QString &name, QString *errorString = nullptr)
    {
        auto generator = GeneratorManager::value(name);
        if (!generator) {
            generator = GeneratorFactory::create(name, errorString);
            if (generator)
                GeneratorManager::append(name, generator);
        }
        return generator;
    }

    /*!
     * \brief addProjectRootItem 添加根數據到TreeView
     * \param aitem
     */
    DPF_INTERFACE(void, addProjectRootItem, QStandardItem *aitem);

    /*!
     * \brief expandedProjectDepth 展开工程子项根据深度
     * \param aitem root节点
     * \param depth 深度
     */
    DPF_INTERFACE(void, expandedProjectDepth, QStandardItem *aitem, int depth);

    /*!
     * \brief expandedProjectAll 展开工程所有子项
     * \param aitem root节点
     */
    DPF_INTERFACE(void, expandedProjectAll, QStandardItem *aitem);

    /*!
     * \brief DPF_INTERFACE
     * insert one item to option dialog with item name and widget.
     */
    DPF_INTERFACE(bool, insertOptionPanel, const QString &itemName, QWidget *panel);

    /*!
     * \brief showProjectOptionsDlg
     * \param show options dialog at panel witch
     */
    DPF_INTERFACE(void, showProjectOptionsDlg, const QString &itemName, const QString &tabName);

    /**
     * @brief showConfigureProjDlg
     * @param outOutputPath return output path
     * @param outBuildTypes return build types user selected
     */
    DPF_INTERFACE(void, showConfigureProjDlg, QString &projectPath);

    /**
     * @brief getDefaultOutputPath
     * @param projectPath
     */
    DPF_INTERFACE(QString, getDefaultOutputPath);

Q_SIGNALS:
    /*!
     * \brief targetExecute 工程目标执行指令, 调用构建系统的入口
     *  当前projectservice不属于构建系统的一部分，需要构建系统支持
     * \param program 程序
     * \param arguments 程序参数
     */
    void targetCommand(const QString &program, const QStringList &arguments);

    /*!
     * \brief projectConfigureDone
     */
    void projectConfigureDone();
};

/* MainWindow codeediter workspace title,
 * use in window service swtich workspace
 */
extern const QString MWCWT_PROJECTS;

} //namespace dpfservice

#endif // PROJECTSERVICE_H
